#include "heater.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    //uint8_t status;

    ot = new OpenTherm(openthermIn, openthermOut);

    // // set the master status (enable CH and DHW)
    // getSetStatus(&status);
}

bool Heater::setTemperature (float setpoint)
{
	uint16_t dataValue = (uint16_t) (setpoint * 256); // the temperature is converted to a 8.8 fixed point

    return ot->setRegister(ID_CONTROL_SETPOINT, dataValue);
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
