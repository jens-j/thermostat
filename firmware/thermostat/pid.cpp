#include "Arduino.h"
#include "common.h"
#include "pid.h"


Pid::Pid (float kP,
          float kI,
          float kD,
          float iMax,
          float input,
          float setpoint,
          float outputMin,
          float outputMax)
{
    changeCoefficients(kP, kI, kD);
    iMax_ = iMax;
    prevInput_ = input;
    initInput_ = input;
    setpoint_ = setpoint;
    outputMin_ = outputMin;
    outputMax_ = outputMax;

    iTerm_ = 0.0;
    prevOutput_ = 0;
}

float Pid::computeStep (float input)
{
    float error = setpoint_ - input;

    iTerm_ += kI_ * error;
    iTerm_ = constrain(iTerm_, outputMin_, outputMax_);

    // float input = kP_ * error + iTerm_ - kD_ * dInput    // p on e & d on m
    float output = -kP_ * (input - initInput_) + iTerm_ - kD_ * (input - prevInput_); // p on m & d on m
    output = constrain(output, outputMin_, outputMax_);

    Serial.println(input);
    Serial.println(initInput_);

    prevInput_ = input;
    prevOutput_ = output;

    return output;
}

void Pid::changeSetpoint (float setpoint);
{
    noInterrupts();
    setpoint_ = setpoint;
    interrupts();
}

void Pid::changeCoefficients (float kP, float kI, float kD)
{
    noInterrupts();
    kP_ = kP;
    kI_ = kI * PID_P; // these expressions are taken out of computeStep()
    kD_ = kD / PID_P; // this avoids performing them every loop iteration
    interrupts();
}

pid_state_log_t Pid::getState ()
{
    pid_state_log_t state = {
        prevInput_,
        prevOutput_,
        setpoint_,
        iTerm_,
        kP_,
        kI_ / PID_P,
        kD_ * PID_P
    };

    return state;
}
