void setup() {

  Serial.begin(115200);
}

uint32_t raw_ADC;
uint32_t voltaje_ADC;

void loop() {

  raw_ADC= analogRead(A0);
  voltaje_ADC = raw_ADC * 3300/1024;
  
  Serial.println(voltaje_ADC);
  delay(1);  
}