#ifndef COMMON_H
#define COMMON_H


/////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////

// pinout
#define OT_INPUT_PIN    3
#define OT_OUTPUT_PIN   12
#define THERMOMETER_PIN 4 // analog pin A4
#define ESP_RX_PIN      11
#define ESP_TX_PIN      2

// number of averaged ADC samples for temperature readings
#define N_ADC_AVG       50

// server address
#define SERVER_IP       "192.168.2.3"
#define CLIENT_IP       "192.168.2.4"
#define SERVER_PORT     8888

// update periods in s
#define P_FAST_TICK     0.02 // 20 ms, buttons and lcd
#define P_SLOW_TICK     1    // 1s, ot message frequency
#define P_PID           5    // 5s, pid updates

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
enum esp_msg_t {PID_UPDATE_LOG = 0,
                SETPOINT_CMD   = 1,
                PID_COEFFS_CMD = 2};

// pid step update log message structure
typedef struct pid_state_log_s {
    float input;
    float output;
    float setpoint;
    float iTerm;
    float kP;
    float kI;
    float kD;
} pid_state_log_t;

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
