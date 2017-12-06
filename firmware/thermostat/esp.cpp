#include <stdint.h>
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
    Serial.println(buf);
    esp_->write(buf);  // connect to the server
    //printReply();
    esp_->write("AT+CIPMODE=1\r\n"); // set to unvarnished transmission mode
    //printReply();
    esp_->write("AT+CIPSEND\r\n"); // start sending data
}

void Esp::logPidState (pid_state_log_t update)
{
    esp_->write((uint8_t) PID_UPDATE_LOG);
    esp_->write((uint8_t*) &update, sizeof(pid_state_log_t));
}

void Esp::printReply () {
    delay(100);
    while (esp_->available()) {
        Serial.write(esp_->read());
        //esp_->read();
    }
}
