#include <SoftwareSerial.h>

SoftwareSerial esp(10, 11); // RX, TX

void setup() {
  pinMode(11, OUTPUT);
  Serial.begin(115200);
  esp.begin(115200);
}

void loop() {
  
  while (esp.available()) {
    Serial.write(esp.read());
  }
  
  while (Serial.available()) {
    esp.write(Serial.read());
  }
  
}
