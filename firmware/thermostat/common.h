#ifndef COMMON_H
#define COMMON_H


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


// number of averaged ADC samples for temperature readings
#define N_ADC_AVG           50

// server address
#define SERVER_IP           "192.168.2.3"
#define CLIENT_IP           "192.168.2.1"
#define SERVER_PORT         8888

// system tick period
#define T_TICK              2 // [ms] needs to be low because it also defines the pwm frequency

// update periods in multiples of the system tick
#define M_UIO               10    // (20 ms) buttons sample and lcd update frequency 
#define M_KEEPALIVE         500   // (1 s)   minimal opentherm message frequency
#define M_PID               2500  // (5 s)   control loop update frequency

// pid coefficients
#define PID_P               20  
#define PID_I               0.05
#define PID_D               0
#define PID_IMAX            100
#define PID_MIN_OUTPUT      10
#define PID_MAX_OUTPUT      90


/////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////

// convert pin number to interrupt number
#define PIN_TO_INT(x) (x == 2 ? 0 : (x == 3 ? 1 : -1))

// check how much free ram is available
int freeRam ();


/////////////////////////////////////////////////
// TYPE DECLARATIONS
/////////////////////////////////////////////////

// esp communication message types
// log: arduino -> server
// cmd: server -> aarduino
enum esp_msg_t {STATE_LOG      = 0,
                SETPOINT_CMD   = 1,
                PID_COEFFS_CMD = 2};

// pid step update log message structure
typedef struct state_log_s {
    float pid_input;
    float pid_output;
    float pid_setpoint;
    float pid_iTerm;
    float pid_kP;
    float pid_kI;
    float pid_kD;
    uint8_t heater_status;
} state_log_t;

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
