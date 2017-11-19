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
    recvFlag = false;
    recvData = 0ULL;
    recvErrorCode = ERR_NONE;
    recvCount = 0;
    recvBusyFlag_ = false;
    recvBuffer_ = 0ULL;
    midBitFlag_ = false;
    recvErrorFlag_ = false;
    recvTimeRef_ = 0UL;
    idleTimeRef_ = 0UL;

    // set IO direction
    pinMode(inputPin_, INPUT);
    pinMode(outputPin_, OUTPUT);
    digitalWrite(outputPin_, HIGH);

    // asign the global object pointer to this instance
    // this allows the global interupt dispatch function to call this instances interrupt handler
    otInstance = this;

    // set up external interrupt
    attachInterrupt(PIN_TO_INT(inputPin_), EXT_ISR, CHANGE);

    // set up WDT interrupt
    wdt_reset();
    WDTCSR |= (1 << WDCE) | (1 << WDE); // change enable
    WDTCSR = (1 << WDP0); // 64 ms
}

void OpenTherm::sendFrame(uint32_t msgType, uint32_t dataId, uint32_t dataValue) {

    char printBuffer[80];
    uint32_t mask = 0x80000000;

    // messages can not be sent less than 100 ms after the last reply was received
    while (millis() - idleTimeRef_ < T_MASTER_IDLE) {}

    // Create the message
    uint32_t msg = (uint32_t) dataValue;
    msg |= (uint32_t) dataId << 16;
    msg |= (uint32_t) msgType << 28;
    msg |= parity32(msg);

    sprintf(printBuffer, "\nsend: 0x%08lx", msg);
    Serial.println(printBuffer);

    // Send start bit
    sendMachesterBit(true);

    // Send message
    while (mask != 0UL) {
        sendMachesterBit(msg & mask);
        mask = mask >> 1;
    }

    // Send stop bit
    sendMachesterBit(true);
}


void OpenTherm::wdtIsr() {

    WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt

    if (recvErrorCode == OT_RECV_ERR_NONE) {
        setRecvError(OT_RECV_ERR_TIMEOUT);
    }

    recvErrorFlag_ = false;
    recvBusyFlag_ = false;
    idleTimeRef_ = millis();
}


// Parse manchester encoded frames from the opentherm interface.
// Each frame consists of a 32 bit message together with positive start and stop bits
void OpenTherm::otIsr() {

    uint32_t t = micros();
    int inputState = digitalRead(_inputPin);

    wdt_reset();

    // If an error happened, discard all data and wait for a timeout
    if (recvErrorFlag_ == true) {
        return;
    }

    // Discard the first transition of the start bit and initialize variables
    if (recvBusyFlag_ == false) {
        // the first edge should always be positive
        if (!inputState) {
            setRecvError(ERR_FIRST_EDGE);
            return;
        }
        recvBusyFlag_ = true;
        recvCount = 0;
        recvBuffer_ = 0x00000000;
        midBitFlag_ = false;
        WDTCSR |= (1<<WDIE); // Enable wdt interrupt
    }
    // First clock transition of the start bit
    else if (recvCount == 0) {

        // this is the frames first bit transition
        recvTimeRef_ = t;
        recvCount++;
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
                setRecvError(ERR_EDGE_EARLY);
            }
        }
        // valid bit transition
        else if (t - recvTimeRef_ < 1150) {

            // shift received bit into the buffer
            recvBuffer_ = inputState ? recvBuffer_ << 1 : (recvBuffer_ << 1) | 1ULL;

            // end of frame
            if (++recvCount == 34) {
                WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt
                recvData = recvBuffer_;
                recvFlag = true;
                recvBusyFlag_ = false;
                idleTimeRef_ = millis();
                return;
            }
            recvTimeRef_ = t;
            midBitFlag_ = false;
        }
        // edge arrived too late
        else {
            setRecvError(ERR_EDGE_LATE);
        }
    }
}

// Send a single machester encoded bit.
void OpenTherm::sendMachesterBit(bool val) {;
    unsigned long t;

    digitalWrite(outputPin_, not val);
    t = micros();
    while (micros() - t < 500) {}
    digitalWrite(outputPin_, val);
    t = micros();
    while (micros() - t < 500) {}
}

void OpenTherm::setRecvError (ot_recv_error_t error)
{
    recvErrorCode = error;
    recvErrorFlag_ = true;
}

// parses a 34 bit frame to a message struct
// returns the bitwise or of all applicable error codes
static uint8_t OpenTherm::parseFrame (uint64_t buf, message_t *msg)
{
    uint8_t error = OT_PARSE_ERR_NONE;

    *msg = parseMessage((uint32_t) buf >> 1);

    if ((buf & 1) != 0ULL) {
        error |= OT_PARSE_ERR_START;
    }
    if ((buf & (1 << 33)) != 0ULL) {
        error |= OT_PARSE_ERR_STOP;
    }
    if (*msg & (1 << 31) != parity32(*msg)) {
        error |= OT_PARSE_ERR_PARITY;
    }
    if (buf & 0xfffffffc00000000ULL != 0ULL) {
        error |= OT_PARSE_ERR_SIZE;
    }

    return error;
}

// parse a 32 bit message into separate fields
message_t OpenTherm::parseMessage (uint32_t buf)
{
    message_t msg = {
        (bool) (buf & (1 << 31)),       // parity bit
        (ot_msg_t) ((msg >> 28) & 0x7); // msg type
        (uint8_t) ((msg >> 16) & 0xff); // data id
        (uint16_t) msg;                 // data value
    }
}

// pretty print an opentherm frame
void OpenTherm::printFrame(uint64_t frameBuf) {

    char cBuffer[80];
    message_t msg;

    sprintf(cBuffer, "raw msg: 0x%08lx%08lx", (uint32_t) (frameBuf >> 32), (uint32_t) frameBuf);
    Serial.println(cBuffer);

    bool errorCode = parseFrame(frameBuf, &msg);
    if (errorCode != OT_PARSE_ERR_NONE) {
        sprintf(cBuffer, "Incorrect message format (0x%x)", errorCode);
        Serial.println(cbuffer);
    }

    Serial.print("msg type:   ");
    Serial.println(OT_MSG_T_STR[msg.msgType]);
    Serial.print("data id:    ");
    Serial.println(msg.dataId);
    Serial.print("data value: ");
    Serial.println(msg.dataValue);
}


// Calculate the even parity bit for a 32 bit vector
// the parity bit is already shifted to the right location (bit 31)
uint32_t OpenTherm::parity32(uint32_t msg) {

    int i;
    uint32_t parity = 0UL;

    for (i = 0; i < 31; i++) { // skip the parity bit itself
        if (msg & 1UL == 1UL) {
            parity = parity ^ 1UL;
        }
        msg = msg >> 1;
    }

   return parity << 31;
}
