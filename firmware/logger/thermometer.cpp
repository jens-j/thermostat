#include "Arduino.h"
#include "thermometer.h"

// Constructor
Thermometer::Thermometer(int pin) {
    inputPin = pin;
    pinMode(inputPin, INPUT);
    analogReference(INTERNAL);
}

// Measure the temperature by taking the average of multiple samples
double Thermometer::getTemperature() {
    int i;
    long sum = 0;

    Serial.println("");
    for (i = 0; i < N_SAMPLES; i++) {
        sum += analogRead(inputPin); 
    }

    return ((double) sum / (double) N_SAMPLES / 1024.0 * 1.1 - 0.5) * 100.0;
}