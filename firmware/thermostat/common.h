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
#define SERVER_IP       "192.168.2.2"
#define CLIENT_IP       "192.168.2.4"
#define SERVER_PORT     8888

// system tick period
#define T_TICK          20   // [ms]

// update periods in multiples of the system tick
#define M_UIO           1    // (20 ms) buttons sample and lcd update frequency 
#define M_KEEPALIVE     50   // (1 s)   minimal opentherm message frequency
#define M_PID           250  // (5 s)   control loop update frequency

// pid coefficients
#define PID_P           20  
#define PID_I           0.05
#define PID_D           0
#define PID_IMAX        100
#define PID_MIN_OUTPUT  10
#define PID_MAX_OUTPUT  90

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
