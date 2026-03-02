#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define I2C_ADDR 0x08
#define LED_PIN LED_BUILTIN

volatile bool i2c_ready = false;
volatile uint8_t last_cmd = 0;

void onReceive(int howMany) {
  while (Wire.available()) {
    last_cmd = Wire.read();   // 0 o 1
    i2c_ready = true;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin(I2C_ADDR);
  Wire.onReceive(onReceive);

  // SLAVE: address primero
  /*bool ok = Wire.begin(8, I2C_SDA, I2C_SCL);
  if (!ok) {
    Serial.println("ERROR iniciando I2C SLAVE");
    while (1) {
      delay(1000);
    }
  }*/

  Wire.onReceive(onReceive);
  Serial.println("ESP32-S3 I2C SLAVE listo");
}

void loop() {
  if (i2c_ready) {
    i2c_ready = false;

    if (last_cmd == 1) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    } else if (last_cmd == 0) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    } else {
      Serial.print("Cmd desconocido: ");
      Serial.println(last_cmd);
    }
  }
}