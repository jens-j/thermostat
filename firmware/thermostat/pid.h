#ifndef PID_H
#define PID_H

#include "common.h"

class Pid {

public:

    // constructor
    Pid (float kP,
         float kI,
         float kD,
         float iMax,
         float input,
         float setpoint,
         float outputMin,
         float outputMax);

    // perform an update of the pid loop
    float computeStep (float input);

    // change the controllers setpoint
    void changeSetpoint (float setpoint);

    // change the controllers p, i and d conefficients
    void changeCoefficients (float kP, float kI, float kD);

    // return a struct containing the controllers state
    pid_state_log_t getState ();

private:

    float kP_;          // proportional coefficient
    float kI_;          // integral coefficient
    float kD_;          // derative coefficient
    float iMax_;        // maximum of the integral
    float iTerm_;       // integral accumulator
    float setpoint_;    // system setpoint
    float prevInput_;   // last pid input
    float initInput_;   // input on startup
    float prevOutput_;  // last pid output
    float outputMin_;   // minimal value for the pid output
    float outputMax_;   // maximal value for the pid output
};

#endif PID_H
