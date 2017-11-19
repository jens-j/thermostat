#import "Arduino.h"
#import "heater.h"
#import "opentherm.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    ot_ = new OpenTherm(openthermIn, openthermOut);
}

bool Heater::setTemperature (float temperature)
{
    ot_->sendFrame(WRITE_DATA, ID_CONTROL_SETPOINT, (uint32_t) temperature);
    return recvReply(T_SLAVE_RESP, true);
}

bool Heater::recvReply(int timeout, bool print)
{
    unsigned long t0 = millis();

    while (millis() - t0 < timeout) {

        // print errors
        if (ot.recvErrorCode != ERR_NONE) {
            code = ot.recvErrorCode;
            ot.recvErrorCode = ERR_NONE;
            ot.recvErrorFlag = false;
            sprintf(cBuffer, "\nrecv error: %s", OT_RECV_ERROR_T_STR[code]);
            Serial.println(cBuffer);
            return false;
        }

        // print messages
        if (ot.recvFlag == true) {
            ot.recvFlag = false;
            OpenTherm::printFrame(ot.recvData);
            return true;
        }
    }

    return false;
}
