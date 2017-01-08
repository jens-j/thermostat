#include <stdint.h>
#include "opentherm.h"


// IO pins
#define outPin 2
#define inPin 3
#define interruptNr 1 // interrupt number for pin 3

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

    Serial.begin(115200);

    // set up external interrupt
    attachInterrupt(interruptNr, extIntDispatch, CHANGE);

    delay(3000);
}


void loop() {
    
    int i;
    int code;
    unsigned long t;
    int ids[] = {ID_DHW_TEMP, ID_DHW_FLOW_RATE, ID_SLAVE_VERSION, ID_RETURN_WATER_TEMP, 
    			 ID_BOILER_WATER_TEMP, ID_FAULT, ID_SLAVE_CONFIG, ID_STATUS};

    for (i = 0; i < sizeof(ids) / 2; i++) {

	    ot.sendFrame(READ_DATA, ids[i], 0);

	    t = millis();

	    while (millis() - t < 2000) {

	        // print errors
	        if (ot.recvErrorCode != ERR_NONE) {
	            code = ot.recvErrorCode;
	            ot.recvErrorCode = ERR_NONE;
	            ot.recvErrorFlag = false;
	            sprintf(cBuffer, "\nerror: %d", code);
	            Serial.println(cBuffer);
	        }

	        // print messages
	        if (ot.recvFlag == true) {
	            ot.recvFlag = false;
	            Serial.println("\nreceived:");
	            OpenTherm::printMsg(ot.recvData);
	            delay(100);
	            break;
	        } 
	    }
	}

	while (true) {}
}
