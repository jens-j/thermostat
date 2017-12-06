
class Thermometer {

public:
    
    // Constructor
    Thermometer(int);

    // Read the current temperature in degrees Celsius
    float getTemperature();

private:

    // The pin connected to the thermometer
    int inputPin;

};