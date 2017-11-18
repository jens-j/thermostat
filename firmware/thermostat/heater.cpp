#import "Arduino.h"
#import "heater.h"
#import "opentherm.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    ot_ = new OpenTherm(openthermIn, openthermOut);
}

bool Heater::setTemperature (float temperature)
{
    long t0 = millis();

    ot_->sendFrame(WRITE_DATA, ID_CONTROL_SETPOINT, (uint32_t) temperature);

    while (millis() - t0 < T_SLAVE_RESP) {

        // print errors
        if (ot.recvErrorCode != ERR_NONE) {
            code = ot.recvErrorCode;
            ot.recvErrorCode = ERR_NONE;
            ot.recvErrorFlag = false;
            sprintf(cBuffer, "\nerror: %d", code);
            Serial.println(cBuffer);
            return false;
        }

        // print messages
        if (ot.recvFlag == true) {
            ot.recvFlag = false;
            Serial.println("\nreceived:");
            OpenTherm::printMsg(ot.recvData);
            return true;
        }
    }

    return false;
}

bool
