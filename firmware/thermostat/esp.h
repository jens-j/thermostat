#ifndef ESP_H
#define ESP_H

#include <SoftwareSerial.h>
#include "pid.h"

class Esp {

public:

    // cosntructor
    Esp (int rx, int tx, Pid *pid);

    // set up the interface with the server
    void initialize ();

    void logState (state_t *state);
    void printReply ();
    void handleCommands (state_t *state);

private:

    Pid *pid_;
    SoftwareSerial *esp_; // serial interface to the ESP-1 module

};

#endif ESP_H
