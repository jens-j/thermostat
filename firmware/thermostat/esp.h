#ifndef ESP_H
#define ESP_H

#include <SoftwareSerial.h>

class Esp {

public:

    // cosntructor
    Esp (int rx, int tx);

    void logPidState (state_log_t update);
    void printReply ();

    SoftwareSerial *esp_; // serial interface to the ESP-1 module

private:

    

};

#endif ESP_H
