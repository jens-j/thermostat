#define N_SAMPLES 10

void setup () 
{
  Serial.begin(9600);
  analogReference(INTERNAL);
}

void loop () 
{ 
  int i;
  int sum = 0;
  for (i = 0; i < N_SAMPLES; i++) {
    sum += analogRead(A4);
  }
  double voltage = (double) sum / (double) N_SAMPLES / 1024.0 * 1.1;
  //double voltage = analogRead(A4) / 1024.0 * 1.1;
  Serial.print("temperature = ");
  Serial.print(voltage * 100.0);
  Serial.println(" C");
  delay(1000);
}
