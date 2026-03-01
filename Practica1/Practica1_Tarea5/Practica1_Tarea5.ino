#include <Arduino.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08

volatile bool i2c_cmd_ready = false;
volatile uint8_t last_cmd = 0;

void receiveEvent(int howMany) {
  while (Wire.available()) {
    last_cmd = Wire.read();
    i2c_cmd_ready = true;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  Serial.println("Nano 33 BLE I2C SLAVE listo");
}

void loop() {

  if (i2c_cmd_ready) {
    i2c_cmd_ready = false;

    if (last_cmd == 1) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("LED ON");
    }
    else if (last_cmd == 0) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED OFF");
    }
  }

  delay(10);
}