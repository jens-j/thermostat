#ifndef COMMON_H
#define COMMON_H

// pinout
#define OT_INPUT_PIN    3
#define OT_OUTPUT_PIN   12
#define THERMOMETER_PIN 1 // analog A1
#define ESP_TX_PIN      11
#define ESP_RX_PIN      2

// constants
#define N_ADC_AVG       50 // number of averaged ADC samples for temperature readings

// conversion
#define PIN_TO_INT(x) (x == 2 ? 0 : (x == 3 ? 1 : -1)) 

// type definitions
typedef bool (*inputFunc) (float);
typedef float (*outputFunc) ();



#endif COMMON_H
