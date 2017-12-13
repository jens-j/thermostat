#ifndef USERIO_H
#define USERIO_H

#include <LiquidCrystal.h> 
#include "common.h"
#include "pid.h"

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
    void update (uint8_t heaterState);

private:

    button_state_t getButtonState_ (); // detects edges
    void printMenu_ (state_log_t state); 

    Pid *pid_;
    LiquidCrystal *lcd_;
    menu_state_t menuState_;
    button_state_t prevButtonState_;
    float setpoint_;

};

#endif USERIO_H