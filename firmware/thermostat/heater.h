#ifndef HEATER_H
#define HEATER_H

#import "opentherm.h"

class Heater {

public:

    // constructor
    Heater (int openthermIn,
            int openthermOut);

    // read the heater water temperature
    bool getTemperature (float *temperature);

    // set the heating system water temperature setpoint
    bool setTemperature (float temperature);

    // set the master enable bits and read the slave status at the same time
    // The master status is fixed and the slave status is returned in slaveStatus
    bool getSetStatus (uint8_t *slaveStatus, uint8_t masterStatus=0x03);
  
    

    // same as above but with specific receive and parse error codes
    bool getTemperatureVerbose (float *temperature, recv_error_t *recvError, 
                                parse_error_t *parseError);

    bool setTemperatureVerbose (float temperature, recv_error_t *recvError, 
                                parse_error_t *parseError);

    bool getSetStatusVerbose (uint8_t *slaveStatus, recv_error_t *recvError, 
                              parse_error_t *parseError, uint8_t masterStatus=0x03);

    // opentherm master interface object pointer
    OpenTherm *ot;

private:

};

#endif HEATER_H
