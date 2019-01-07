#ifndef ESP_H
#define ESP_H

#include <SoftwareSerial.h>
#include "opentherm.h"
#include "pid.h"

class Esp {

public:

    // cosntructor
    Esp (int rx, int tx, Pid *pid);

    // set up the interface with the server
    void initialize ();

    // log the state of the controller 
    void logState (state_t *state);

    // combined dispatcher for receive and parse error log messages
    void logOtError(recv_error_t *recvError, parse_error_t *parseError);

    // log the arduino was reset or powered up
    void logReset ();

    // print the reply from the esp if any
    void printReply ();

    // poll the esp for incoming commands from the server and handle them
    void handleCommands (state_t *state);

private:

    Pid *pid_;
    SoftwareSerial *esp_; // serial interface to the ESP-1 module

};

#endif ESP_H
