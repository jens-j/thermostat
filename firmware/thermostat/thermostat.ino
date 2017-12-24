#include <SoftwareSerial.h> // arduino libraries must be imported from the .ino file
#include <LiquidCrystal.h>  // 
#include "TimerOne.h"
#include "common.h"
#include "esp.h"
#include "heater.h"
#include "opentherm.h"
#include "thermometer.h"
#include "pid.h"
#include "userio.h"

// interface objects
Heater *heater = new Heater(OT_INPUT_PIN, OT_OUTPUT_PIN);
Thermometer *thermometer = new Thermometer();
Esp *esp;
Pid *pid;
UserIo *userIo;

// isr global variables
int uioCount = 0;
int tempReadCount = 0;
int tempDispCount = 0;
int cmdCount = 0;
int pidCount = 0;

// isr <-> main loop communication variables
volatile bool uioFlag = false;
volatile bool tempReadFlag = false;
volatile bool tempDispFlag = false;
volatile bool cmdFlag = false;
volatile bool keepaliveFlag = false;
volatile bool pidFlag = false;

// main loop global variables
state_t *state = new state_t();

// increase counters and check if task should be scheduled
void TIMER1_ISR ()
{
    if (++uioCount >= M_UIO) {
        uioCount = 0;
        uioFlag = true;
    }
    if (++tempReadCount >= M_TEMP_READ) {
        tempReadCount = 0;
        tempReadFlag = true;
    }
    if (++tempDispCount >= M_TEMP_DISP) {
        tempDispCount = 0;
        tempDispFlag = true;
    }
    if (++cmdCount >= M_CMD) {
        cmdCount = 0;
        cmdFlag = true;
    }
    if (heater->ot->checkKeepAlive() >= M_KEEPALIVE) {
        heater->ot->resetKeepAlive();
        keepaliveFlag = true;
    }
    if (++pidCount >= M_PID) {
        pidCount = 0;
        pidFlag = true;
    }
}

void setup ()
{

    char cBuf[40];
    uint8_t heaterStatus;
    bool success = false;

    //delay(2000);

    Serial.begin(115200);
    Serial.println(F("init"));

    // set up the timer1 interrupt (needed for lcb backlight pwm)
    Timer1.initialize(T_TICK * 1E3);

    thermometer->update();

    pid = new Pid(PID_P,
                  PID_I, 
                  PID_D,
                  PID_IMAX,
                  thermometer->getTemperature(),  // initial input
                  20, // setpoint
                  PID_MIN_OUTPUT, 
                  PID_MAX_OUTPUT);
    userIo = new UserIo(pid);
    esp = new Esp(ESP_RX_PIN, ESP_TX_PIN, pid);
    esp->initialize();

    // // try to read the initial status of the boiler
    // while (!success) {
    //     success = heater->getSetStatus(&heaterStatus);
    //     if (success) {
    //         sprintf(cBuf, "status: 0x%x", heaterStatus);
    //         Serial.println(cBuf);
    //     } else {
    //         Serial.println("could not read heater status");
    //     }
    //     delay(1000);
    // }

    Serial.println(F("start\n"));
    Timer1.attachInterrupt(TIMER1_ISR);

    keepaliveFlag = true;
    pidFlag = true;
}

void loop ()
{
    // char cBuf[80];
    bool success;
    float roomTemperature;
    float boilerTemperature;
    
    if (uioFlag) {
        uioFlag = false;

        // read the buttons and update the lcd
        userIo->update(state);


    } else if (tempReadFlag) {
        tempReadFlag = false;

        // take a thermometer reading and update the average
        thermometer->update();

    } else if (tempDispFlag) {
        tempDispFlag = false;

        // update the temperature display
        state->temperature = thermometer->getTemperature();    

    } else if (cmdFlag) {
        cmdFlag = false;

        // poll messages and handle any commands
        esp->handleCommands(state);

    } else if (keepaliveFlag) {
        keepaliveFlag = false;

        // read the heater status
        success = heater->getSetStatus(&state->heater_status);
        if (success) {
            Serial.print(F("status: 0x"));
            Serial.println(state->heater_status, HEX);
        } else {
            Serial.println(F("read error"));
        }
    } 
    else if (pidFlag) {
        pidFlag = false;

        // performa a pid update
        boilerTemperature = pid->computeStep(state->temperature);

        // write water temperature to the heater
        success = heater->setTemperature(boilerTemperature);
        if (!success) {
            Serial.println(F("write error"));
        }

        // log the state to the server
        pid->getState(&(state->pid));
        esp->logState(state);
    }
}
