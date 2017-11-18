#ifndef ESP_H
#define ESP_H

#include <SoftwareSerial.h>

class Esp {

public:

    // cosntructor
    Esp (int rx, int tx);

    void logPidState (pid_state_log_t update);
    void printReply ();

private:

    SoftwareSerial *esp_; // serial interface to the ESP 01 module

};

#endif ESP_H
