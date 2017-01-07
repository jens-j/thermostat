
void setup() {
  Serial.begin(14400);
  Serial1.begin(14400);
}

void loop() {
  
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  
  while (Serial.available()) {
    Serial1.write(Serial.read());
  }
  
}
