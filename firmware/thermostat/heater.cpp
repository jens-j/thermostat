#include "heater.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    //uint8_t status;

    ot = new OpenTherm(openthermIn, openthermOut);

    // // set the master status (enable CH and DHW)
    // getSetStatus(&status);
}

bool Heater::getTemperature (float *temperature)
{
    bool success;
    uint16_t readValue;

    success = ot->getRegister(ID_BOILER_WATER_TEMP, &readValue);
    *temperature = (float) (readValue) / 256.0;

    return success;
}

bool Heater::setTemperature (float setpoint)
{
    uint16_t dataValue = (uint16_t) (setpoint * 256); // the temperature is converted to a 8.8 fixed point

    return ot->setRegister(ID_CONTROL_SETPOINT, dataValue);
}

bool Heater::getSetStatus (uint8_t *slaveStatus, uint8_t masterStatus) 
{   
    bool success;
    uint16_t readValue;
    uint16_t writeValue = ((uint16_t) masterStatus) << 8;

    success = ot->getRegister(ID_STATUS, &readValue, writeValue=writeValue);
    *slaveStatus = (uint8_t) readValue;

    return success;
}
