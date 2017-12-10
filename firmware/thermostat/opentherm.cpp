#include <stdint.h>
#include <avr/wdt.h>
#include "Arduino.h"
#include "common.h"
#include "opentherm.h"

OpenTherm *otInstance = NULL;

// opentherm message receive timeout
ISR(WDT_vect) {
    otInstance->wdtIsr();
}

// external interrupt on the opentherm input pin
void EXT_ISR() {
    otInstance->otIsr();
}

OpenTherm::OpenTherm(int inPin, int outPin) {

    // store configuration
    inputPin_ = inPin;
    outputPin_ = outPin;

    // initialize variables
    recvFlag_           = false;
    recvData_           = 0ULL;
    recvErrorCode_      = OT_RECV_ERR_NONE;
    recvCount_          = 0;
    recvBusyFlag_       = false;
    recvBuffer_         = 0ULL;
    midBitFlag_         = false;
    recvErrorFlag_      = false;
    recvTimeRef_        = 0UL;
    idleTimeRef_        = 0UL;
    keepAliveCounter_   = 0;

    // set IO direction
    pinMode(inputPin_, INPUT);
    pinMode(outputPin_, OUTPUT);
    digitalWrite(outputPin_, HIGH);

    // asign the global object pointer to this instance
    // this allows the global interupt dispatch function to call this instances interrupt handler
    otInstance = this;

    // set up external interrupt
    delayMicroseconds(20); // wait for the ot interface to settle
    EIFR = (1 << PIN_TO_INT(inputPin_));
    attachInterrupt(PIN_TO_INT(inputPin_), EXT_ISR, CHANGE);

    // set up WDT interrupt
    wdt_reset();
    WDTCSR |= (1 << WDCE) | (1 << WDE); // change enable
    WDTCSR = (1 << WDP0); // 64 ms
}

bool OpenTherm::setRegister(uint8_t dataId, uint16_t dataValue) 
{
    uint64_t frameBuf;
    message_t message;
    ot_recv_error_t recvError;
    uint8_t parseError;
    int recvCount;

    sendFrame(WRITE_DATA, dataId, dataValue);
    recvError = recvReply(&frameBuf, &recvCount);
    parseError = parseFrame(frameBuf, &message);

    if (recvError != OT_RECV_ERR_NONE) {
        Serial.println('a');
        Serial.println(recvError);
        return false;
    } else if (parseError != 0) {
        Serial.println('b');
        Serial.println(parseError);
        return false;
    } else if (message.msgType != WRITE_ACK) {
        Serial.println('c');
        Serial.println(message.msgType);
        return false;
    }  else {
        return true;
    }
}

bool OpenTherm::getRegister(uint8_t dataId, uint16_t *readValue, uint16_t writeValue) 
{
    uint64_t frameBuf;
    message_t message;
    ot_recv_error_t recvError;
    uint8_t parseError;
    int recvCount;

    sendFrame(READ_DATA, dataId, writeValue);
    recvError = recvReply(&frameBuf, &recvCount);
    parseError = parseFrame(frameBuf, &message);
    *readValue = message.dataValue;

    if (recvError != OT_RECV_ERR_NONE) {
        Serial.println('a');
        Serial.println(recvError);
        return false;
    } else if (parseError != 0) {
        Serial.println('b');
        Serial.println(parseError);
        return false;
    } else if (message.msgType != READ_ACK) {
        Serial.println('c');
        Serial.println(message.msgType);
        return false;
    }  else {
        return true;
    }
}

void OpenTherm::sendFrame(int msgType, uint8_t dataId, uint16_t dataValue) 
{

    char printBuffer[100];
    uint32_t mask = 0x80000000;

    // messages can not be sent less than 100 ms after the last reply was received
    while (millis() - idleTimeRef_ < T_MASTER_IDLE) {}

    // Create the message
    uint32_t msg = (uint32_t) dataValue;
    msg |= (uint32_t) dataId << 16;
    msg |= (uint32_t) msgType << 28;
    msg |= parity32(msg);

    // sprintf(printBuffer, "\nsend: 0x%08lx (id = %d)", msg, dataId);
    // Serial.println(printBuffer);

    // diable interrupts to avoid false positive recv interrupts (crosstalk)
    // and to avoid other interrupts to mess up the timing of the frame
    detachInterrupt(PIN_TO_INT(inputPin_));

    // Send start bit
    sendMachesterBit_(true);

    // Send message
    while (mask != 0UL) {
        sendMachesterBit_(msg & mask);
        mask = mask >> 1;
    }

    // Send stop bit
    sendMachesterBit_(true);

    resetKeepAlive();

    EIFR = (1 << PIN_TO_INT(inputPin_));
    attachInterrupt(PIN_TO_INT(inputPin_), EXT_ISR, CHANGE);
}

// receive the reply to a read or write request. return the receive error code.
// either the receive or receive error flag should go up before the maximal resonse time elapses.
ot_recv_error_t OpenTherm::recvReply(uint64_t *frameBuf, int *n)
{
    ot_recv_error_t errorCode;
    unsigned long t0 = millis();

    *n = 0;
    *frameBuf = 0ULL;

    recvFlag_ = false;
    recvErrorCode_ = OT_RECV_ERR_NONE;

    while (millis() - t0 < T_SLAVE_RESP) {

        // print errors
        if (recvErrorCode_ != OT_RECV_ERR_NONE) {
            errorCode = recvErrorCode_;
            recvErrorCode_ = OT_RECV_ERR_NONE;
            *n = recvCount_;
            recvErrorFlag_ = false;
            return errorCode;
        }

        // print messages
        if (recvFlag_ == true) {
            *n = recvCount_;
            *frameBuf = recvData_;
            recvFlag_ = false;
            return OT_RECV_ERR_NONE;
        }
    }

    return OT_RECV_ERR_TIMEOUT;
}

void OpenTherm::wdtIsr() {

    WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt

    if (recvErrorCode_ == OT_RECV_ERR_NONE) {
        setRecvError_(OT_RECV_ERR_INCOMPLETE);
    }

    recvBusyFlag_ = false;
    idleTimeRef_ = millis();
}


// Parse manchester encoded frames from the opentherm interface.
// Each frame consists of a 32 bit message together with positive start and stop bits
void OpenTherm::otIsr() {

    uint32_t t = micros();
    wdt_reset();

    delayMicroseconds(5); // this is probably not necessary
    int inputState = digitalRead(inputPin_);

    // If an error happened, discard all data and wait for a timeout
    if (recvErrorFlag_ == true) {
        return;
    }

    // Discard the first transition of the start bit and initialize variables
    if (recvBusyFlag_ == false) {
        // the first edge should always be rising
        if (!inputState) {
            pinMode(7, OUTPUT);
            digitalWrite(7, HIGH);
            digitalWrite(7, LOW);
            setRecvError_(OT_RECV_ERR_FIRST_EDGE);
            return;
        }
        recvFlag_ = false;
        recvData_ = 0ULL;
        recvErrorCode_ = OT_RECV_ERR_NONE;
        recvCount_ = 0;

        recvBusyFlag_ = true;  
        recvBuffer_ = 0x00000000;
        midBitFlag_ = false;
        
        WDTCSR |= (1<<WDIE); // Enable wdt interrupt
    }
    // First clock transition of the start bit
    else if (recvCount_ == 0) {

        // this is the frames first bit transition
        recvTimeRef_ = t;
        recvCount_++;
        recvBuffer_ = inputState ? recvBuffer_ << 1 : (recvBuffer_ << 1) | 1ULL; // shift received bit into the buffer
    }
    // All other transitions
    else {
        // check for half cycle transitions
        if (t - recvTimeRef_ < 900) {
            if (midBitFlag_ == false) {
                midBitFlag_ = true;
                return;
            } else {
                setRecvError_(OT_RECV_ERR_EDGE_EARLY);
            }
        }
        // valid bit transition
        else if (t - recvTimeRef_ < 1150) {

            // shift received bit into the buffer
            recvBuffer_ = inputState ? recvBuffer_ << 1 : (recvBuffer_ << 1) | 1ULL;

            // check for end of frame
            if (++recvCount_ == 34) {
                WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt
                recvData_ = recvBuffer_;
                recvFlag_ = true;
                recvBusyFlag_ = false;
                idleTimeRef_ = millis();
                return;
            }
            recvTimeRef_ = t;
            midBitFlag_ = false;
        }
        // edge arrived too late
        else {
            setRecvError_(OT_RECV_ERR_EDGE_LATE);
        }
    }
}

// Send a single machester encoded bit.
void OpenTherm::sendMachesterBit_(bool val) {;
    unsigned long t;

    digitalWrite(outputPin_, not val);
    t = micros();
    while (micros() - t < 500) {}
    digitalWrite(outputPin_, val);
    t = micros();
    while (micros() - t < 500) {}
}

void OpenTherm::setRecvError_ (ot_recv_error_t errorCode)
{
    recvErrorCode_ = errorCode;
    recvErrorFlag_ = true;
}

// parses a 34 bit frame to a message struct
// returns the bitwise or of all applicable error codes
uint8_t OpenTherm::parseFrame (uint64_t frameBuf, message_t *msg)
{
    char cBuf[40];
    uint8_t error = OT_PARSE_ERR_NONE;
    uint32_t msgBuf = (uint32_t) (frameBuf >> 1);
    *msg = parseMessage(msgBuf);

    if (!(frameBuf & 0x0000000000000001)) {
        error |= OT_PARSE_ERR_START;
    }
    if (!(frameBuf & 0x0000000200000000)) {
        error |= OT_PARSE_ERR_STOP;
    }
    if (frameBuf &   0xfffffffc00000000) {
        error |= OT_PARSE_ERR_SIZE;
    }
    if ((msgBuf & 0x80000000UL) != parity32(msgBuf)) {
        error |= OT_PARSE_ERR_PARITY;
    }

    return error;
}

// parse a 32 bit message into separate fields
message_t OpenTherm::parseMessage (uint32_t buf)
{
    message_t msg = {
        (bool)      (buf & 0x80000000UL),        // parity bit
        (uint8_t)  ((buf & 0x70000000UL) >> 28), // msg type
        (uint8_t)  ((buf & 0x00ff0000UL) >> 16), // data id
        (uint16_t)  (buf & 0x0000ffffUL)         // data value
    };

    return msg;
}

// pretty print an opentherm frame
void OpenTherm::printFrame(uint64_t frameBuf) {

    char cBuffer[100];
    message_t msg;

    sprintf(cBuffer, "raw msg: 0x%08lx%08lx", (uint32_t) (frameBuf >> 32), (uint32_t) frameBuf);
    Serial.println(cBuffer);

    uint8_t errorCode = parseFrame(frameBuf, &msg);
    if (errorCode != OT_PARSE_ERR_NONE) {
        sprintf(cBuffer, "incorrect message format (0x%x)", errorCode);
        Serial.println(cBuffer);
    }


    Serial.print("msg type: ");
    Serial.println(OT_MSG_T_STR[msg.msgType]);
    Serial.print("data id:    ");
    Serial.println(msg.dataId);
    sprintf(cBuffer, "data value: %d (0x%x)", msg.dataValue, msg.dataValue);
    Serial.print(cBuffer);
}


// Calculate the even parity bit for a 32 bit vector
// the parity bit is already shifted to the right location (bit 31) in the result
uint32_t OpenTherm::parity32(uint32_t msg) {

    int i;
    uint32_t parity = 0UL;

    for (i = 0; i < 31; i++) { // skip the parity bit itself at postion 31
        parity = parity ^ (msg & 1UL);
        msg = msg >> 1;
    }

   return parity << 31;
}

int OpenTherm::checkKeepAlive () 
{
    return ++keepAliveCounter_;
} 

void OpenTherm::resetKeepAlive ()
{   
    keepAliveCounter_ = 0;
}