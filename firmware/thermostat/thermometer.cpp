#import "Arduino.h"
#import "common.h"
#import "thermometer.h"

Thermometer::Thermometer () {

    pinMode(THERMOMETER_PIN, INPUT);
    initialized_ = false;
}

void Thermometer::update () 
{
    float val = readThermometer_();

    if (initialized_) {
        averageTemperature_ = (1 - TMP_C_IIR) * averageTemperature_ + TMP_C_IIR * val;
    } else {
        averageTemperature_ = val;
        initialized_ = true;
    }
}

float Thermometer::readThermometer_ () {
    int i;
    uint32_t sum = 0;

    for (i = 0; i < TMP_ADC_AVG; i++) {
      sum += analogRead(THERMOMETER_PIN);
    }
    float voltage = (float) sum / (float) TMP_ADC_AVG / 1024.0 * 5.0;

    return voltage * 100.0;
}
