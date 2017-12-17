#ifndef USERIO_H
#define USERIO_H

#include <LiquidCrystal.h> 
#include "Arduino.h"
#include "common.h"
#include "pid.h"

static byte upArrow[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b01110,
  0b01110,
  0b00000
};

enum button_state_t {
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_SELECT,
    BTN_NONE
};

enum menu_state_t {
    MENU_FRONT,
    MENU_SETPOINT
};


class UserIo
{

public:

    // constructor
    UserIo (Pid *pid);

    // read the buttons and update the lcd
    void update (state_t *state);

    button_state_t getButtonState (); 
    button_state_t getButtonEdge (); 

    void printMenu (state_t *state); 

private:

    Pid *pid_;
    LiquidCrystal *lcd_;
    menu_state_t menuState_;
    button_state_t prevButtonState_;
    float setpoint_;
    state_t prevState_;

};

#endif USERIO_H