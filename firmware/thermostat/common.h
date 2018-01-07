#ifndef COMMON_H
#define COMMON_H

#include "Arduino.h"

/////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////

// pinout
#define OT_INPUT_PIN        3
#define OT_OUTPUT_PIN       12
#define THERMOMETER_PIN     4 // analog pin A4
#define ESP_RX_PIN          11
#define ESP_TX_PIN          2
#define BUTTONS_PIN         1 // analog pin A1
#define LCD_DB4_PIN         4
#define LCD_DB5_PIN         5
#define LCD_DB6_PIN         6
#define LCD_DB7_PIN         7
#define LCD_RS_PIN          8
#define LCD_ENABLE_PIN      9
#define LCD_BACKLIGHT_PIN   10

// thermometer parameters
#define TMP_ADC_AVG         50   // number of averaged ADC samples for temperature readings
#define TMP_C_IIR           0.02 // moving average coefficient

// server address
#define SERVER_IP           "192.168.2.3"
#define CLIENT_IP           "192.168.2.1"
#define SERVER_PORT         8888

// system tick period
#define T_TICK              2 // [ms] needs to be low because it also defines the pwm frequency

// update periods in multiples of the system tick
#define M_UIO               10    // (20 ms)  buttons sample and lcd update frequency 
#define M_TEMP_READ         50    // (100 ms) temperature filter update
#define M_TEMP_DISP         250   // (500 ms) temperature display updates
#define M_KEEPALIVE         500   // (1 s)    minimal opentherm message frequency
#define M_CMD               500   // (1 s)    esp cmd poll
#define M_PID               2500  // (5 s)    control loop update frequency

// pid coefficients
#define PID_P               20 
#define PID_I               0.01
#define PID_D               0
#define PID_IMAX            100
#define PID_MIN_OUTPUT      10
#define PID_MAX_OUTPUT      90
#define PID_I_BAND          1

// other temperature control parameters
#define CTRL_HYSTERESIS     1 // [C] heater is enabled if themperature drop this much below the setpoint


/////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////

// convert pin number to interrupt number
#define PIN_TO_INT(x) (x == 2 ? 0 : (x == 3 ? 1 : -1))

#define ROUND(x) (x - (int) x < 0.5 ? (int) x : (int) x + 1) 

// check how much free ram is available
int freeRam ();

/////////////////////////////////////////////////
// TYPE DECLARATIONS
/////////////////////////////////////////////////

// esp communication message types
// log: arduino -> server
// cmd: server -> arduino
// 8 bits wide when used as message header
enum esp_msg_t {STATE_LOG           = 0,
                OT_RECV_ERROR_LOG   = 1,
                OT_PARSE_ERROR_LOG  = 2,
                RESET_LOG           = 3,
                SETPOINT_CMD        = 4,
                PID_COEFFS_CMD      = 5};

// pid step update log message structure
typedef struct pid_state_s {
    float input;
    float output;
    float setpoint;
    float errorSum;
    float kP;
    float kI;
    float kD;
} pid_state_t;

// state update log message structure
typedef struct state_s {
    pid_state_t pid;
    uint8_t heaterStatus;
    float heaterTemperature;
    float roomTemperature;
    bool otError;
} state_t;

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
