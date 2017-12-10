#include <SoftwareSerial.h>

SoftwareSerial esp(11, 2); // RX, TX

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  esp.begin(9600);

  esp.write("AT+CIPSTART=\"TCP\",\"192.168.2.2\",8888\r\n");
  delay(1000);
  esp.write("AT+CIPMODE=1\r\n");
  delay(1000);
  esp.write("AT+CIPSEND\r\n");
}

void loop() {
  
  while (esp.available()) {
    Serial.write(esp.read());
  }
  
  while (Serial.available()) {
    esp.write(Serial.read());
  }
  
}
