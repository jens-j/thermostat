#import "Arduino.h"
#import "heater.h"
#import "opentherm.h"

Heater::Heater (int openthermIn, int openthermOut)
{
    ot_ = new OpenTherm(openthermIn, openthermOut);
}

bool Heater::setTemperature (float temperature)
{
    return true;
}
