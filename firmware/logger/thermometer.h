#define N_SAMPLES 100

class Thermometer {

public:
    
    // Constructor
    Thermometer(int);

    // Read the current temperature in degrees Celsius
    double getTemperature();

private:

    // The pin connected to the thermometer
    int inputPin;

};