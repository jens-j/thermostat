#include <stdint.h>
#include "Arduino.h"
#include "common.h"
#include "esp.h"
#include "pid.h"

Esp::Esp(int rx, int tx)
{
    // set arduino pin directions
    pinMode(rx, INPUT);
    pinMode(tx, OUTPUT);

    // setup the serial interface with the esp
    esp_ = new SoftwareSerial(rx, tx);
    esp_->setTimeout(1);
    esp_->begin(9600);
}

// set up the interface with the server
void Esp::initialize () 
{
    char buf[40];

    esp_->write("AT+CIPCLOSE\r\n");
    delay(50);
    sprintf(buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
    esp_->write(buf);  // connect to the server
    // delay(1000);
    // esp_->write("AT+CIPMODE=1\r\n"); // set to unvarnished transmission mode
    // delay(1000);
    // esp_->write("AT+CIPSEND\r\n"); // start sending data
}

void Esp::logState (state_t *update)
{
    esp_->write((uint8_t) STATE_LOG);
    esp_->write((uint8_t*) update, sizeof(state_t));
}

void Esp::printReply ()
{
    delay(100);
    while (esp_->available()) {
        Serial.write(esp_->read());
    }
}

void Esp::handleCommands () 
{
    while (Serial.available()) {
        Serial.println(esp_->read());
    }

    // String cmd = "";
    //cmd = Serial.readString();
    // if (cmd != "") {
    //     Serial.print("cmd: ");
    //     Serial.println(cmd);
    // }
}
