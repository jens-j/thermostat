#include "Arduino.h"
#include "common.h"
#include "esp.h"

Esp::Esp(int rx, int tx)
{
    char buf[100];

    // set arduino pin directions
    pinMode(rx, INPUT);
    pinMode(tx, OUTPUT);

    // setup the serial interface with the esp
    esp_ = new SoftwareSerial(rx, tx);
    esp_->begin(9600);

    // set up ther interface with the server
    sprintf(buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
    esp_->write(buf);  // connect to the server
    esp_->write("AT+CIPMODE=1\r\n"); // set to unvarnished transmission mode
    esp_->write("AT+CIPSEND\r\n"); // start sending data
}

void Esp::logPidUpdate (pid_update_log_t update)
{

}
