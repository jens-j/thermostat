#include <SoftwareSerial.h>

SoftwareSerial esp(10, 11); // RX, TX

void setup() {
  pinMode(11, OUTPUT);
  Serial.begin(9600);
  esp.begin(9600);
}

void loop() {
  
  while (esp.available()) {
    Serial.write(esp.read());
  }
  
  while (Serial.available()) {
    esp.write(Serial.read());
  }
  
}
