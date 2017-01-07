#include <stdint.h>
#include "opentherm.h"


// IO pins
#define outPin 2
#define inPin 3
#define interruptNr 1 // interrupt number for pin 3

#define testPin 4


// variables
char cBuffer[80];
OpenTherm ot = OpenTherm(inPin, outPin, interruptNr);


// opentherm message receive timeout
ISR(WDT_vect) {
    ot.wdtIsr();
}

void extIntDispatch() {

    digitalWrite(testPin, HIGH);
    digitalRead(inPin);
    digitalWrite(testPin, LOW);
    
    ot.recvIsr(digitalRead(inPin));
}


void setup() {

    pinMode(testPin, OUTPUT);

    Serial.begin(115200);

    // set up external interrupt
    attachInterrupt(interruptNr, extIntDispatch, CHANGE);
}


void loop() {
    
    int code;
    unsigned long t;

    ot.sendFrame(READ_DATA, ID_SLAVE_VERSION, 0);

    t = millis();

    while (millis() - t < 2000) {

        // print errors
        if (ot.recvErrorCode != ERR_NONE) {
            code = ot.recvErrorCode;
            ot.recvErrorCode = ERR_NONE;
            ot.recvErrorFlag = false;
            sprintf(cBuffer, "error: %d", code);
            Serial.println(cBuffer);
        }

        // print messages
        if (ot.recvFlag == true) {
            ot.recvFlag = false;
            sprintf(cBuffer, "recv: 0x%08lx%08lx (%d)", 
                (uint32_t) (ot.recvData >> 32), (uint32_t) ot.recvData, ot.recvCount);
            Serial.println(cBuffer);
        } 
    }
}
