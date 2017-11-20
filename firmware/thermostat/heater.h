#ifndef HEATER_H
#define HEATER_H

#import "opentherm.h"

class Heater {

public:

    // constructor
    Heater (int openthermIn,
            int openthermOut);

    // set the heating system water temperature
    bool setTemperature (float temperature);

private:

    // opentherm master inteface object pointer
    OpenTherm *ot_;
};

#endif HEATER_H
