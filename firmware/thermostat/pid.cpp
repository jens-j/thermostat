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

    timestamp_ = millis();
    iTerm_ = 0.0;
    prevOutput_ = 0;
}

float Pid::computeStep (float input)
{
    float prevTimestamp = timestamp_;
    float dt = (millis() - timestamp_) * 1E3;
    float error = setpoint_ - input;
    float dInput = (input - prevInput_) / dt;

    iTerm_ += kI_ * error * dt;
    iTerm_ = constrain(iTerm_, outputMin_, outputMax_);

    // p on e & d on m
    float output = kP_ * error + iTerm_ - kD_ * dInput;  

    // p on m & d on m
    // float output = -kP_ * (input - initInput_) + iTerm_ - kD_ * dInput; // p on m & d on m
    
    output = constrain(output, outputMin_, outputMax_);

    prevInput_ = input;
    prevOutput_ = output;
    timestamp_ = prevTimestamp;

    return output;
}

void Pid::changeSetpoint (float setpoint)
{
    setpoint_ = setpoint;
}

void Pid::changeCoefficients (float kP, float kI, float kD)
{
    kP_ = kP;
    kI_ = kI;
    kD_ = kD; 
}

pid_state_log_t Pid::getState ()
{
    pid_state_log_t state = {
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
