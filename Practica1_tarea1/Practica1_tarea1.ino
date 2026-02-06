void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

int raw_ADC;
// the loop function runs over and over again forever
void loop() {

  raw_ADC= analogRead(A0);
  
  Serial.println(raw_ADC);
  delay(1);  
}