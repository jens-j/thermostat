#ifndef THERMOMETER_H
#define THERMOMETER_H


class Thermometer {

public:

    // constructor
    Thermometer ();

    // sample the thermometer and update the average
    void update ();

    // return the temperature average
    float getTemperature () {return averageTemperature_;}

private:

    // read the current temperature in degrees Celsius. takes the mean of multiple samples
    float readThermometer_ ();

    float averageTemperature_;
    bool initialized_;

};

#endif THERMOMETER_H
