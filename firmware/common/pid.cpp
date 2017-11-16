#include "Arduino.h"
#include "TimerOne.h"
#include "pid.h"

Pid *pidInstance = NULL;

void ISR_DISP ()
{
    pidInstance->handleInterrupt();
}

Pid::Pid (float period, float kP, float kI, float kD, float iMax, inputFunc fI, outputFunc fO)
{
    period_ = period;
    kP_ = kP;
    kI_ = kI;
    kD_ = kD;
    iMax_ = iMax;
    fI_ = fI;
    fO_ = fO;


    // set up the timer interrupt throught the static interface
    Timer1.initialize(period);
    Timer1.attachInterrupt(ISR_DISP);

    // asign the global object pointer to this instance
    // this allows the global interupt dispatch function to call this instances interrupt handler
    pidInstance = this;
}

void Pid::handleInterrupt ()
{

}


void Pid::changeCoefficients (float kP, float kI, float kD)
{
    kP_ = kP;
    kI_ = kI;
    kD_ = kD;
}
