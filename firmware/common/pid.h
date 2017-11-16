#ifndef PID_H
#define PID_H

typedef void (*inputFunc) (float);
typedef float (*outputFunc) ();

class Pid {

public:

    Pid (float period, float kP, float kI, float kD, float iMax, inputFunc fI, outputFunc fO);
    void handleInterrupt ();
    void changeCoefficients (float kP, float kI, float kD);

private:

    float period_;  // the update perdiod
    float kP_;      // proportional coefficient
    float kI_;      // integral coefficient
    float kD_;      // derative coefficient
    float iMax_;    // maximum of the integral
    inputFunc fI_;  // system input callback function pointer (pid output)
    outputFunc fO_; // system output callback function pointer (pid input)
};

#endif PID_H
