#include <LiquidCrystal.h>
#include "Arduino.h"
#include "TimerOne.h"
#include "common.h"
#include "pid.h"
#include "opentherm.h"
#include "userio.h"

UserIo::UserIo (Pid *pid)
{
    pid_ = pid;
    menuState_ = MENU_FRONT;
    prevButtonState_ = getButtonState();
    pinMode(BUTTONS_PIN, INPUT);
    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
    Timer1.pwm(LCD_BACKLIGHT_PIN, 256);
    lcd_ = new LiquidCrystal(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN);
    lcd_->begin(16, 2);
    lcd_->createChar(0, upArrow);
    lcd_->createChar(1, degreeSign);
}

void UserIo::update (state_t *state)
{
    // collect state information
    button_state_t buttonState = getButtonEdge();

    bool change = false;
    menu_state_t newMenuState = menuState_;
    float prevSetpoint = setpoint_;

    // process button input
    switch (menuState_) {
        case MENU_FRONT:
            if (buttonState == BTN_SELECT) {
                setpoint_ = ROUND(state->pid.setpoint * 2) / 2.0;
                newMenuState = MENU_SETPOINT;
            }
            break;

         case MENU_SETPOINT:
            if (buttonState == BTN_UP) {
                setpoint_ += 0.5;
            } else if (buttonState == BTN_DOWN) {
                setpoint_ -= 0.5;
            } else if (buttonState == BTN_SELECT) {
                pid_->changeSetpoint(setpoint_);
                state->pid.setpoint = setpoint_;
                newMenuState = MENU_FRONT;
            }
    }

    if (setpoint_ != prevSetpoint || newMenuState != menuState_ || 
            memcmp(&prevState_, state, sizeof(state_t))) {
        change = true;
    }

    menuState_ = newMenuState;
    memcpy(&prevState_, state, sizeof(state_t));

    if (change) {
        printMenu(state);
    }   
}

void UserIo::printMenu (state_t *state) 
{
    char cBuf[10];

    switch (menuState_) {
        case MENU_FRONT:
            lcd_->setCursor(0, 0);
            lcd_->print(F(" "));
            dtostrf(state->room_temperature, 5, 2, cBuf);
            lcd_->print(cBuf);
            lcd_->print(F(" ["));
            dtostrf(state->pid.setpoint, 4, 1, cBuf);
            lcd_->print(cBuf);
            lcd_->print(F("]"));
            lcd_->write((unsigned char) 1);
            lcd_->print(F("C"));

            lcd_->setCursor(0, 1);
            if (state->heater_status & STATUS_FLAME) {
                lcd_->write((unsigned char) 0);
            } else {
                lcd_->write(' ');
            }
            dtostrf(state->heater_temperature, 5, 2, cBuf);
            lcd_->print(cBuf);
            lcd_->print(F(" ["));
            dtostrf(ROUND(state->pid.output), 4, 1, cBuf);
            lcd_->print(cBuf);
            lcd_->print(F("]"));
            lcd_->write((unsigned char) 1);
            lcd_->print(F("C"));
            break;

        case MENU_SETPOINT:
            lcd_->setCursor(0, 0);
            lcd_->print(F("setpoint:           "));
            lcd_->setCursor(0, 1);
            dtostrf(setpoint_, 5, 2, cBuf);
            lcd_->print(cBuf);
            lcd_->write((unsigned char) 1);
            lcd_->print(F("C             "));
    }
}

button_state_t UserIo::getButtonState () 
{
    //analogReference(DEFAULT);
    int value = analogRead(BUTTONS_PIN);
    //analogReference(INTERNAL);

    if (value < 73) {
        return BTN_RIGHT;
    } else if (value <= 237) {
        return BTN_UP;
    } else if (value <= 417) {
        return BTN_DOWN;
    } else if (value <= 623) {
        return BTN_LEFT;
    } else if (value < 883) {
        return BTN_SELECT;
    } else {
        return BTN_NONE;
    }
}

button_state_t UserIo::getButtonEdge () 
{
    button_state_t buttonState = getButtonState();

    if (prevButtonState_ != BTN_NONE) {
        prevButtonState_ = buttonState;
        return BTN_NONE;
    } else {
        prevButtonState_ = buttonState;
        return buttonState;
    }
}

