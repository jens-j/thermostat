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

static byte degreeSymbol[8] = {
    0b00110,
    0b01001,
    0b01001,
    0b00110,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

static byte degreeCelsiusSymbol[8] = {
    0b10000,
    0b00110,
    0b01001,
    0b01000,
    0b01000,
    0b01000,
    0b01001,
    0b00110
};

static byte emptySymbol[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

static byte singleExclamation[8] = {
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00000,
    0b00100,
    0b00000
};

static byte doubleExclamation[8] = {
    0b00000,
    0b01010,
    0b01010,
    0b01010,
    0b01010,
    0b00000,
    0b01010,
    0b00000
};


enum symbol_t {
    SYMBOL_UPARROW,
    SYMBOL_DEGREE,
    SYMBOL_DEGREECELSIUS,
    SYMBOL_EMPTY,
    SYMBOL_SINGLEEXCLAMATION,
    SYMBOL_DOUBLEEXCLAMATION
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

    LiquidCrystal *lcd_;

    // constructor
    UserIo (Pid *pid);

    // read the buttons and update the lcd
    void update (state_t *state);

    // check which button in now pressed
    button_state_t getButtonState ();

    // check if a button edge occured since last call
    button_state_t getButtonEdge (); 

    // print the thermometer menu
    void printMenu (state_t *state); 

private:

    Pid *pid_;
    
    menu_state_t menuState_;
    button_state_t prevButtonState_;
    float setpoint_;
    state_t prevState_;
    symbol_t errorIndicator_;
    bool errorSticky_;

};

#endif USERIO_H