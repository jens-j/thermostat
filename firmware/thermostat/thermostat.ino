#include "common.h"
#include "heater.h"
#include "thermometer.h"
#include "pid.h"

Heater heater = Heater(OT_INPUT_PIN, OT_OUTPUT_PIN);
Thermometer thermometer = Thermometer(THERMOMETER_PIN, N_ADC_AVG);
Pid pid = Pid(1.0,   // update period
              1.0,   // kP
              0.1,   // kI
              0.0,   // kD
              10.0,  // Imax
              20.0,  // setpoint
              setTemperature,
              readTemperature);

void setup ()
{
    Serial.begin(115200);
}

void loop ()
{

}

// super lame callback dispatchers
bool setTemperature (float temperature) {heater.setTemperature(temperature);}
float readTemperature () {thermometer.readTemperature();}
