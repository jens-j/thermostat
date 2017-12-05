#include <math.h>
#include "heater.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    ot = new OpenTherm(openthermIn, openthermOut);
}

bool Heater::setTemperature (float setpoint)
{
    return ot->setRegister(ID_CONTROL_SETPOINT, (uint16_t) round(setpoint));
}

bool Heater::getStatus (uint8_t *status) 
{   
    uint16_t dataValue;
    bool success;

    success = ot->getRegister(ID_STATUS, &dataValue);
    *status = (uint8_t) dataValue;

    return success;
}
