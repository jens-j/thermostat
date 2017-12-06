#include "common.h"
#include "opentherm.h"

OpenTherm ot = OpenTherm(OT_INPUT_PIN, OT_OUTPUT_PIN);


void setup() {

    Serial.begin(115200);
    delay(2000);
}


void loop() {

    char cBuf[80];
    uint8_t status;
    bool success;

    success = heater.getStatus(&status);

    if (success) {
        sprintf(cBuf, "status: 0x%x");
        Serial.println(cBuf);
    } else {
        Serial.println("recveive error");
    }

    delay(1000);

}
