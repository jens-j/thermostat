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
    setpoint_ = setpoint;
    outputMin_ = outputMin;
    outputMax_ = outputMax;

    iTerm_ = 0.0;
    prevOutput_ = 0;
}

float Pid::computeStep (float input)
{
    float error = setpoint_ - input;
    float dInput = input - prevInput_;

    iTerm_ += kI_ * error;
    iTerm_ = constrain(iTerm_, outputMin_, outputMax_);

    // float input = kP_ * error + iTerm_ - kD_ * dInput    // p on e & d on m
    float output = -kP_ * dInput + iTerm_ - kD_ * dInput; // p on m & d on m
    output = constrain(output, outputMin_, outputMax_);

    prevInput_ = input;
    prevOutput_ = output;

    return output;
}

void Pid::changeCoefficients (float kP, float kI, float kD)
{
    kP_ = kP;
    kI_ = kI * PID_P; // these expressions are taken out of computeStep()
    kD_ = kD / PID_P; // this avoids performing them every loop iteration
}

pid_update_log_t Pid::getState ()
{
    pid_update_log_t state = {
        prevInput_,
        prevOutput_,
        setpoint_,
        iTerm_,
        kP_,
        kI_,
        kD_
    };

    return state;
}
