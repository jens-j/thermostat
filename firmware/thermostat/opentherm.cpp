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
    inputPin = inPin;
    outputPin = outPin;

    // initialize variables
    recvFlag = false;
    recvData = 0ULL;
    recvErrorCode = ERR_NONE;
    recvBuffer = 0ULL;
    recvCount = 0;
    recvErrorFlag = false;

    // set IO direction
    pinMode(inputPin, INPUT);
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, HIGH);

    // asign the global object pointer to this instance
    // this allows the global interupt dispatch function to call this instances interrupt handler
    otInstance = this;

    // set up external interrupt
    attachInterrupt(PIN_TO_INT(inPin), EXT_ISR, CHANGE);

    // set up WDT interrupt
    wdt_reset();
    WDTCSR |= (1 << WDCE) | (1 << WDE); // change enable
    WDTCSR = (1 << WDP0); // 64 ms
}

void OpenTherm::sendFrame(uint32_t msgType, uint32_t dataId, uint32_t dataValue) {

    char printBuffer[80];
    uint32_t mask = 0x80000000;

    // Create the message
    uint32_t msg = (uint32_t) dataValue;
    msg |= (uint32_t) dataId << 16;
    msg |= (uint32_t) msgType << 28;
    msg |= parity32(msg) << 31;

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

    // Disable wdt interrupt
    WDTCSR &= ~(1<<WDIE);

    recvErrorCode = ERR_TIMEOUT;
    recvErrorFlag = false;
    recvBusyFlag = false;
}


// Parse manchester encoded frames from the opentherm interface.
// Each frame consists of a 32 bit message together with positive start and stop bits
void OpenTherm::otIsr() {

    uint32_t t = micros();
    int inputState = digitalRead(inputPin);
    wdt_reset();

    //Serial.println(digitalRead(inputPin));

    // If an error happened, discard all data and wait for a timeout
    if (recvErrorFlag == true) {
        recvCount++;
        return;
    }

    // Discard the first transition of the start bit and initialize variables
    if (recvBusyFlag == false) {
        // the first edge should always be positive
        if (!inputState) {
            recvErrorCode = ERR_FIRST_EDGE;
            return;
        }
        //Serial.println(1);
        recvBusyFlag = true;
        recvCount = 0;
        recvBuffer = 0x00000000;
        midBitFlag = false;
        WDTCSR |= (1<<WDIE); // Enable wdt interrupt
    }
    // First clock transition of the start bit
    else if (recvCount == 0) {

        // this is the messages first bit transition
        recvTimeRef = t;
        recvCount++;
        if (inputState) {
            recvBuffer = recvBuffer << 1;
        } else {
            recvBuffer = (recvBuffer << 1) | 1UL;
        }
    }
    // All other transitions
    else {
        // check for half cycle transitions
        if (t - recvTimeRef < 900) {
            if (midBitFlag == false) {
                midBitFlag = true;
                return;
            } else {
                recvErrorFlag = true;
                recvErrorCode = ERR_EDGE_EARLY;
            }
        }
        // valid bit transition
        else if (t - recvTimeRef < 1150) {

            if (inputState) {
                recvBuffer = recvBuffer << 1;
            } else {
                recvBuffer = (recvBuffer << 1) | 1UL;
            }

            if (++recvCount == 34) {
                WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt
                recvFlag = true;
                recvBusyFlag = false;
                recvData = recvBuffer;
                recvMsg = parseMessage((uint32_t) (recvBuffer >> 1));
                return;
            }
            recvTimeRef = t;
            midBitFlag = false;
        }
        // edge arrived too late
        else {
            recvErrorFlag = true;
            recvErrorCode = ERR_EDGE_LATE;
        }
    }
}


// Send a single machester encoded bit.
void OpenTherm::sendMachesterBit(bool val) {;
    unsigned long t;

    digitalWrite(outputPin, not val);
    t = micros();
    while (micros() - t < 500) {}
    digitalWrite(outputPin, val);
    t = micros();
    while (micros() - t < 500) {}
}


message_t OpenTherm::parseMessage (uint32_t buf)
{
    message_t msg = {
        (bool) (buf & (1 << 31)),       // parity bit
        (MsgType) ((msg >> 28) & 0x7);  // msg type
        (uint8_t) ((msg >> 16) & 0xff); // data id
        (uint16_t) msg;                 // data value
    }
}


// pretty print an opentherm frame
void OpenTherm::printMsg(message_t msg) {

    char cBuffer[80];

    int msgType = (msg >> 29) & 0x7;
    int dataId = (msg >> 17) & 0xff;
    int dataValue = msg >> 1;

    sprintf(cBuffer, "raw msg: 0x%08lx%08lx", (uint32_t) (msg >> 32), (uint32_t) msg);
    Serial.println(cBuffer);

    if (!(msg & 0x200000000ULL))
        Serial.println("missing start bit!");
    if (!(msg & 1))
        Serial.println("missing stop bit!");
    if (OpenTherm::parity32(msg) == 0)
        Serial.println("Parity error!");

    Serial.print("msg type:   ");
    Serial.println(MSG_TYPE[msgType]);
    Serial.print("data id:    ");
    Serial.println(dataId);
    Serial.print("data value: ");
    Serial.println(dataValue);
}


// Calculate the even parity bit for a 32 bit vector
uint32_t OpenTherm::parity32(uint32_t x) {

    int i;
    uint32_t parity = 0UL;

    for (i = 0; i < 32; i++) {
        if (x & 1UL == 1UL) {
            parity = parity ^ 1UL;
        }
        x = x >> 1;
    }
   return parity;
}
