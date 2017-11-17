#include "Arduino.h"
#include "TimerOne.h"
#include "common.h"
#include "pid.h"

Pid *pidInstance = NULL;

void TIMER1_ISR ()
{
    pidInstance->handleInterrupt();
}

Pid::Pid (float period,
          float kP,
          float kI,
          float kD,
          float iMax,
          float setpoint,
          inputFunc fI,
          outputFunc fO)
{
    period_ = period;
    kP_ = kP;
    kI_ = kI;
    kD_ = kD;
    iMax_ = iMax;
    fI_ = fI;
    fO_ = fO;
    iTerm_ = 0.0;
    setpoint_ = setpoint;
    prevError_ = setpoint_ - fO_();

    // set up the timer interrupt throught the static interface
    Timer1.initialize(period);
    Timer1.attachInterrupt(TIMER1_ISR);

    // asign the global object pointer to this instance
    // this allows the global interupt dispatch function to call this instances interrupt handler
    pidInstance = this;
}

void Pid::handleInterrupt ()
{
    float output = fO_();
    float error = setpoint_ - output;
    iTerm_ += kI_ * error;
    float input = kP_ * error + iTerm_ + kD_ * (error - prevError_);
    fI_(input);
    prevError_ = error;
}

void Pid::changeCoefficients (float kP, float kI, float kD)
{
    kP_ = kP;
    kI_ = kI;
    kD_ = kD;
}
