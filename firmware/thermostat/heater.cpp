#include <math.h>
#include "heater.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    uint8_t status;

    ot = new OpenTherm(openthermIn, openthermOut);

    // set the master status (enable CH and DHW)
    getSetStatus(&status);
}

bool Heater::setTemperature (float setpoint)
{
    return ot->setRegister(ID_CONTROL_SETPOINT, (uint16_t) round(setpoint));
}

bool Heater::getSetStatus (uint8_t *slaveStatus) 
{   
	bool success;
    uint16_t readValue;
    uint16_t writeValue = 0x0300; // enable CH and DHW

    success = ot->getRegister(ID_STATUS, &readValue, writeValue=writeValue);
    *slaveStatus = (uint8_t) readValue;

    return success;
}
