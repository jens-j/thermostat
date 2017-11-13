#include <SoftwareSerial.h>
#include "thermometer.h"

SoftwareSerial esp(11, 2); // RX, TX
Thermometer thermometer(A4);

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  esp.begin(9600);

  esp.write("AT+CIPSTART=\"TCP\",\"192.168.2.3\",8888\r\n");  // connect to the server
  printReply();
  esp.write("AT+CIPMODE=1\r\n"); // set to unvarnished transmission mode
  printReply();
  esp.write("AT+CIPSEND\r\n"); // start sending data
}

void loop() {
  
    // read temperature
    float temperature = thermometer.getTemperature();
    uint8_t *p = (uint8_t*) &temperature;

    // send the temperature to the server 
    esp.write((uint8_t*) &temperature, 4);

    delay(2000);
}

void printReply() {
  delay(100);
  while (esp.available()) {
    Serial.write(esp.read());
  }    
}
