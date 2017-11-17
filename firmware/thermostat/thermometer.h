#ifndef THERMOMETER_H
#define THERMOMETER_H

class Thermometer {

public:

    // constructor
    Thermometer (int pin,
                 int nAverages);

    // read the current temperature in degrees Celsius. takes the mean of multiple samples
    float readTemperature ();

private:

    int pin_;       // analog pin connected to the thermometer
    int nAverages_; // the number of samples that are averaged for one temperature reading

};

#endif THERMOMETER_H
