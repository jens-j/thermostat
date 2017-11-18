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
Pid pid = Pid(1.0,   // kP
              0.5,   // kI
              0.5,   // kD
              100.0, // Imax
              thermometer.readTemperature(), // initial input
              20.0,  // setpoint
              0.0,   // minimal output
              100.0);// maximal output

// main loop variables
float msg_tick_counter = 0;
float pid_tick_counter = 0;

void TIMER1_ISR ()
{

    // read buttons and update lcd

    msg_tick_counter += USER_INPUT_P;
    pid_tick_counter += USER_INPUT_P;

    if (msg_tick_counter >= MSG_P) {
        msg_tick_counter -= MSG_P;

        // handle commands
    }

    if (pid_tick_counter >= PID_P) {
        pid_tick_counter -= PID_P;

        // pid update
        bool success = heater.setTemperature(pid.computeStep(thermometer.readTemperature()));
    }
}

void setup ()
{
    Serial.begin(115200);

    // set up the timer1 interrupt
    Timer1.initialize(USER_INPUT_P);
    Timer1.attachInterrupt(TIMER1_ISR);
}

void loop ()
{

}
