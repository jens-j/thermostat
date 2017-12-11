#include "Arduino.h"
#include "common.h"
#include "userio.h"

UserIo::UserIo (Pid *pid)
{
    pid_ = pid;

    pinMode(BUTTONS_PIN, INPUT);
    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
    analogWrite(LCD_BACKLIGHT_PIN, 64);
    lcd_ = Lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN);
    prevButtonState_ = getButtonState_();
}

void UserIo::update (uint8_t heaterState)
{
    // collect state information
    state_log_t state;
    state.heaterState = heaterState;
    pid_.getState(&state);
    button_state_t buttonState = getButtonState_();

    // process button input
    switch (menuState_) {
        case MENU_FRONT:
            if (buttonState == BTN_SELECT) {
                newSetpoint_ = state.pid_setpoint;
                menuState_ = MENU_SETPOINT;
            }
            break;

         case MENU_SETPOINT:
            if (buttonState == BTN_UP) {
                newSetpoint_ += 0.5;
            } else if (buttonState == BTN_DOWN) {
                newSetpoint_ -= 0.5;
            } else if (buttonState == BTN_SELECT) {
                pid_->changeSetpoint(newSetpoint_);
                state.pid_setpoint = newSetpoint_;
                menuState_ = MENU_FRONT;
            }
    }

    printMenu_(state);
}

void UserIo::printMenu_ (state_log_t state) 
{
    char cBuf[20];
    char flame = (state.heater_status & 0x08) ? '*' : ' ';

    switch (menuState_) {
        case MENU_FRONT:
            lcd_.setCursor(0, 0);
            sprintf(cBuf, "%2.1f [%2.1f] C       ", state.pid_input, pid_setpoint);
            lcd_.print(cBuf);
            lcd_.setCursor(0, 1);
            sprintf(cBuf, "%2.1f C [%c]          ", state.pid_output, flame);
            lcd_.print(cBuf);
            break;

        case MENU_SETPOINT:
            lcd_.setCursor(0, 0);
            lcd_.print("setpoint:           ");
            lcd_.setCursor(0, 1);
            sprintf(cBuf, "%2.1f C              ", newSetpoint_);
            lcd_.print(cBuf);
    }
}

// detects edges
button_state_t UserIo::getButtonState_ () 
{
    button_state_t buttonState;
    int value = analogRead(BUTTONS_PIN);

    if (value < 73) {
        buttonState = BTN_RIGHT;
    } else if (value <= 237) {
        buttonState = BTN_UP;
    } else if (value <= 417) {
        buttonState = BTN_DOWN;
    } else if (value <= 623) {
        buttonState = BTN_LEFT;
    } else if (value < 883) {
        buttonState = BTN_SELECT;
    } else {
        buttonState = BTN_NONE;
    }

    if (prevButtonState_ != BTN_NONE) {
        prevButtonState_ = buttonState;
        return BTN_NONE;
    } else {
        prevButtonState_ = buttonState;
        return buttonState;
    }
}