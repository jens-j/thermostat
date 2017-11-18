#ifndef COMMON_H
#define COMMON_H


/////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////

// pinout
#define OT_INPUT_PIN    3
#define OT_OUTPUT_PIN   12
#define THERMOMETER_PIN 1 // analog A1
#define ESP_TX_PIN      11
#define ESP_RX_PIN      2

// number of averaged ADC samples for temperature readings
#define N_ADC_AVG       50

// server address
#define SERVER_IP       "192.168.2.3"
#define SERVER_PORT     8888

// update periods in s
#define USER_INPUT_P    0.02 // buttons and lcd
#define MSG_P           1.0  // esp interface
#define PID_P           5.0  // pid updates

/////////////////////////////////////////////////
// CONVERSIONS
/////////////////////////////////////////////////

#define PIN_TO_INT(x) (x == 2 ? 0 : (x == 3 ? 1 : -1))


/////////////////////////////////////////////////
// TYPE DECLARATIONS
/////////////////////////////////////////////////

// esp communication message types
// log: arduino -> server
// cmd: server -> aarduino
enum MSG_TYPE {PID_UPDATE_LOG,
               SETPOINT_CMD,
               PID_COEFFS_CMD};

// pid step update log message structure
typedef struct pid_update_log_s {
    float input;
    float output;
    float setpoint;
    float iTerm;
    float kP;
    float kI;
    float kD;
} pid_update_log_t;

// system setpoint update message
typedef struct setpoint_cmd_s {
    float setPoint;
} setpoint_cmd_t;

// set pid coefficients command message structure
typedef struct set_coeffs_cmd_s {
    float kP;
    float kI;
    float kD;
} set_coeffs_cmd_t;


#endif COMMON_H