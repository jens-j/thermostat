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

    // receive the reply to a read or write request. return succes/failure
    // timeout - waiting time in ms
    // debug   - debug print on/off
    bool recvReply(int timeout, bool print);

    // opentherm master inteface object pointer
    OpenTherm *ot_;
};

#endif HEATER_H
