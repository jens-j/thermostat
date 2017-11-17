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

    OpenTherm *ot_; // opentherm master inteface object pointer

};

#endif HEATER_H
