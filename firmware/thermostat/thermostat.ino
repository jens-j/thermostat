#include <SoftwareSerial.h> // this can only be done from the .ino file
#include "TimerOne.h"
#include "common.h"
#include "esp.h"
#include "heater.h"
#include "opentherm.h"
#include "thermometer.h"
#include "pid.h"

// interface objects
Esp *esp = new Esp(ESP_RX_PIN, ESP_TX_PIN);
Heater *heater = new Heater(OT_INPUT_PIN, OT_OUTPUT_PIN);
Thermometer *thermometer = new Thermometer(THERMOMETER_PIN, N_ADC_AVG);
Pid *pid;

// isr tick counters 
int uioCount = 0;
int pidCount = 0;

// isr <-> main loop communication variables
volatile bool uioFlag = false;
volatile bool keepaliveFlag = false;
volatile bool pidFlag = false;

// increase counters and check if task should be scheduled
void TIMER1_ISR ()
{
    if (++uioCount >= P_UIO) {
        uioCount = 0;
        uioFlag = true;
    }
    if (heater->ot->checkKeepAlive() >= P_KEEPALIVE) {
        heater->ot->resetKeepAlive();
        keepaliveFlag = true;
    }
    if (++pidCount >= P_PID) {
        pidCount = 0;
        pidFlag = true;
    }
}

void setup ()
{
    delay(2000);

    Serial.begin(115200);
    Serial.println("initializing");

    pid = new Pid(20.0,  // kP
                  0.1,   // kI
                  0.0,   // kD
                  100.0, // Imax
                  thermometer->readTemperature(),  // initial input
                  21.0,  // setpoint
                  0.0,   // minimal output
                  100.0);// maximal output

    // set up the timer1 interrupt
    Timer1.initialize(P_TICK * 1E3);
    Timer1.attachInterrupt(TIMER1_ISR);
}

void loop ()
{
    char cBuf[80];
    bool success;
    float roomTemperature;
    float boilerTemperature;
    pid_state_log_t pidState;
    uint8_t heaterStatus;

    if (uioFlag == true) {
        uioFlag = false;

        // sample buttons

    } else if (pidFlag == true) {
        pidFlag = false;

        // perform a pid update 
        roomTemperature = thermometer->readTemperature();

        Serial.print("room: ");
        Serial.print(roomTemperature);
        Serial.println(" C");

        boilerTemperature = pid->computeStep(roomTemperature);

        Serial.print("heater: ");
        Serial.print(boilerTemperature);
        Serial.println(" C");

        success = heater->setTemperature(boilerTemperature);
        if (!success) {
            Serial.println("write error");
        }

        // log the state to the server
        pidState = pid->getState();
        esp->logPidState(pidState);

    } else if (keepaliveFlag == true) {
        keepaliveFlag = false;

        success = heater->getStatus(&heaterStatus);
        if (success) {
            sprintf(cBuf, "status: 0x%x", heaterStatus);
            Serial.println(cBuf);
        } else {
            Serial.println("read error");
        }

    } 
}
