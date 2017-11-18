#include <SoftwareSerial.h> // this can only be done from the .ino file
#include "TimerOne.h"
#include "common.h"
#include "esp.h"
#include "heater.h"
#include "thermometer.h"
#include "pid.h"

// interface objects
Esp esp = Esp(ESP_RX_PIN, ESP_TX_PIN);
Heater heater = Heater(OT_INPUT_PIN, OT_OUTPUT_PIN);
Thermometer thermometer = Thermometer(THERMOMETER_PIN, N_ADC_AVG);
Pid *pid;

// main loop variables
float msg_tick_counter = 0;
float pid_tick_counter = 0;
pid_state_log_t pidState;

void TIMER1_ISR ()
{

    // read buttons and update lcd

    msg_tick_counter += P_USER_INPUT;
    pid_tick_counter += P_USER_INPUT;

    if (msg_tick_counter >= P_MSG) {
        msg_tick_counter -= P_MSG;

        // ot keepalive

        // handle commands
    }

    if (pid_tick_counter >= P_PID) {
        pid_tick_counter -= P_PID;

        // pid update
        bool success = heater.setTemperature(pid->computeStep(thermometer.readTemperature()));
        if (success) {
            pidState = pid->getState();
            esp.logPidState(pidState);
            Serial.print("input, output = ");
            Serial.print(pidState.input);
            Serial.print(", ");
            Serial.println(pidState.output);
        } else {
            Serial.println("Could not set heater temperature");
        }
    }
}

void setup ()
{
    Serial.begin(115200);

    Serial.println("initializing");

    float temp = thermometer.readTemperature();
    Serial.println(temp);
    pid = new Pid(1.0,   // kP
                  0.5,   // kI
                  0.5,   // kD
                  100.0, // Imax
                  temp,  // initial input
                  30.0,  // setpoint
                  0.0,   // minimal output
                  100.0);// maximal output

    // set up the timer1 interrupt
    Timer1.initialize(USER_INPUT_P * 1E6);
    Timer1.attachInterrupt(TIMER1_ISR);
}

void loop ()
{

}
