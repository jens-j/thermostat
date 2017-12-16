#import "Arduino.h"
#import "thermometer.h"

Thermometer::Thermometer (int pin, int nAverages) {
    pin_ = pin;
    nAverages_ = nAverages;
    pinMode(pin, INPUT);
    analogReference(INTERNAL);
}

float Thermometer::readTemperature () {
    int i;
    uint32_t sum = 0;
    
    // analogReference(INTERNAL);
    // delay(20);

    for (i = 0; i < nAverages_; i++) {
      sum += analogRead(pin_);
    }
    float voltage = (float) sum / (float) nAverages_ / 1024.0 * 1.1;

    return voltage * 100.0;
}
