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

    // read the slave status word
    bool getStatus (uint8_t *status);

    // opentherm master interface object pointer
    OpenTherm *ot;

private:

};

#endif HEATER_H
