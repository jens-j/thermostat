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
    float output;
    float dt = (millis() - timestamp_) / 1E3;
    timestamp_ = millis();
    float error = setpoint_ - input;
    float dInput = (input - prevInput_) / dt;

    iTerm_ += kI_ * error * dt;
    iTerm_ = constrain(iTerm_, 0, outputMax_);

    // p on e & d on m
    output = kP_ * error + iTerm_ - kD_ * dInput;  

    // p on m & d on m
    // float output = -kP_ * (input - initInput_) + iTerm_ - kD_ * dInput; // p on m & d on m
    
    // positive pid output should result in boiler water which is warmer than the room temerature
    output = output + input; 
    output = constrain(output, outputMin_, outputMax_);

    prevInput_ = input;
    prevOutput_ = output;

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

void Pid::getState (pid_state_t *state)
{
    state->input = prevInput_;
    state->output = prevOutput_;
    state->setpoint = setpoint_;
    state->iTerm = iTerm_;
    state->kP = kP_;
    state->kI = kI_;
    state->kD = kD_;
}
