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
