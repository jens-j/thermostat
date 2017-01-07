#include <avr/wdt.h>
#include <stdint.h>
#include "Arduino.h"
#include "opentherm.h"


OpenTherm::OpenTherm(int inputPin, int outPin, int interruptNr) {

    // store configuration
    inputPin = inputPin;
    outputPin = outPin;
    interruptNumber = interruptNr;

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

    sprintf(printBuffer, "send: 0x%08lx", msg);
    Serial.println(printBuffer);

    //detachInterrupt(interruptNumberr);

    // Send start bit
    sendMachesterBit(true);

    // Send message
    while (mask != 0UL) {
        if (msg & mask) {
            sendMachesterBit(1);
        } else {
            sendMachesterBit(0);
        }
        mask = mask >> 1;
    }

    // Send stop bit
    sendMachesterBit(true);

    //attachInterrupt(interruptNumberr, recvIsr, CHANGE);
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
void OpenTherm::recvIsr(int inputState) {


    uint32_t t = micros();
    wdt_reset();

    //Serial.println(digitalRead(inputPin));

    // If an error happened, discard all data and wait for a timeout
    if (recvErrorFlag == true) {
        //Serial.println(2);
        recvCount++;
        return;
    }

    // Discard the first transition of the start bit and initialize variables
    if (recvBusyFlag == false) {
        // the first edge should always be positive
        if (digitalRead(inputPin) == LOW) {
            //Serial.println(0);
            return;
        }
        //Serial.println(1);
        recvBusyFlag = true;
        recvCount = 0;
        recvBuffer = 0x00000000;
        midBitFlag = false;
        recvErrorCode = ERR_NONE;
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

            digitalWrite(testPin, HIGH);
            digitalRead(inPin);
            digitalWrite(testPin, LOW);

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
void OpenTherm::sendMachesterBit(int val) {;
    digitalWrite(outputPin, val);
    delayMicroseconds(500);
    digitalWrite(outputPin, not val);
    delayMicroseconds(500);
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