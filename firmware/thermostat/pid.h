#ifndef PID_H
#define PID_H

#include "common.h"

class Pid {

public:

    Pid (float period,
         float kP,
         float kI,
         float kD,
         float iMax,
         float setpoint,
         inputFunc fI,
         outputFunc fO);
         
    void handleInterrupt ();
    void changeSetpoint (float setpoint) {setpoint_ = setpoint;}
    void changeCoefficients (float kP, float kI, float kD);

private:

    float period_;      // the update period
    float kP_;          // proportional coefficient
    float kI_;          // integral coefficient
    float kD_;          // derative coefficient
    float iMax_;        // maximum of the integral
    float iTerm_;       // integral accumulator
    inputFunc fI_;      // system input callback function pointer (pid output)
    outputFunc fO_;     // system output callback function pointer (pid input)
    float setpoint_;    // system setpoint
    float prevError_;   // error meaured in previous loop iteration
};

#endif PID_H
