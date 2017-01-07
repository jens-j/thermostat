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
    ot.recvIsr();
}


void setup() {

    pinMode(testPin, OUTPUT);

    Serial.begin(115200);

    // set up external interrupt
    attachInterrupt(interruptNr, extIntDispatch, CHANGE);

    delay(2000);
}


void loop() {
    
    int code;
    unsigned long t;

    ot.sendFrame(READ_DATA, ID_SLAVE_VERSION, 0);

    t = millis();

    while (millis() - t < 4000) {

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
            Serial.println("\nreceived:");
            OpenTherm::printMsg(ot.recvData);
        } 
    }
}
