#ifndef ESP_H
#define ESP_H

#include <SoftwareSerial.h>
#include "pid.h"

class Esp {

public:

    // cosntructor
    Esp (int rx, int tx);

    // set up the interface with the server
    void initialize ();

    void logState (state_t *update);
    void printReply ();
    void handleCommands ();


    SoftwareSerial *esp_; // serial interface to the ESP-1 module

private:

    Pid *pid_;

};

#endif ESP_H
