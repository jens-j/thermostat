#include <stdint.h>
#include "Arduino.h"
#include "common.h"
#include "opentherm.h"
#include "esp.h"
#include "pid.h"

Esp::Esp(int rx, int tx, Pid *pid)
{
    // set arduino pin directions
    pinMode(rx, INPUT);
    pinMode(tx, OUTPUT);

    pid_ = pid;

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
    delay(50);
    esp_->write("AT+CIPMODE=1\r\n"); // set to transparent transmission mode
    delay(50);
    esp_->write("AT+CIPSEND\r\n"); // start sending data
}

void Esp::logState (state_t *state)
{
    esp_->write((uint8_t) STATE_LOG);
    esp_->write((uint8_t*) state, sizeof(state_t));
}

void Esp::logOtError(recv_error_t *recvError, parse_error_t *parseError)
{
    if (recvError->errorFlags != OT_RECV_ERR_NONE) {
        esp_->write((uint8_t) OT_RECV_ERROR_LOG);
        esp_->write((uint8_t*) recvError, sizeof(recv_error_t));
    }
    if (parseError->errorType != OT_PARSE_ERR_NONE) {
        esp_->write((uint8_t) OT_PARSE_ERROR_LOG);
        esp_->write((uint8_t*) parseError, sizeof(parse_error_t));
    }
}

void Esp::printReply ()
{
    delay(100);
    while (esp_->available()) {
        Serial.write(esp_->read());
    }
}

void Esp::handleCommands (state_t *state) 
{
    int i;
    char buffer[64];
    int recvCount = esp_->readBytes(buffer, 64);

    if (recvCount > 0) {

        switch (buffer[0]) {

            case SETPOINT_CMD:
                if (recvCount != 1 + sizeof(setpoint_cmd_t)) {
                    Serial.print(F("(esp) invalid setpoint command "));
                    Serial.println(recvCount);
                } else {
                    float *setpoint;
                    setpoint = (float*) &(buffer[1]);
                    Serial.print(F("(esp) setpoint command: "));
                    Serial.println(*setpoint, 5);
                    pid_->changeSetpoint(*setpoint);
                    state->pid.setpoint = *setpoint;
                }
                break;

            case PID_COEFFS_CMD:
                Serial.println(F("(esp) pid coefficients command"));

                break;

            default:
                Serial.print(F("(esp) unknown command: "));
                Serial.println(buffer[0]);
        }
    }
}
