#include "Arduino.h"
#include "common.h"
#include "thermometer.h"

// Constructor
Thermometer::Thermometer(int pin) {
    inputPin = pin;
    pinMode(inputPin, INPUT);
    analogReference(INTERNAL);
}

// Measure the temperature by taking the average of multiple samples
float Thermometer::getTemperature() {
    int i;
    long sum = 0;

    for (i = 0; i < N_ADC_AVG; i++) {
        sum += analogRead(inputPin); 
    }

    return ((float) sum / (float) N_ADC_AVG / 1024.0 * 1.1) * 100.0;
}