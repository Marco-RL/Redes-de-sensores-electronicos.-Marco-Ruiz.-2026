#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR 0x08

uint8_t led_cmd = 0;
bool led_cmd_pending = false;

void shell();

void setup() {
  Serial.begin(115200);

  Wire.begin();            // MASTER
  Wire.setClock(100000);

  Serial.println("Nano 33 BLE I2C MASTER listo (manda 0/1)");
}

void loop() {
  if (Serial.available()) {
    shell();
  }

  if (led_cmd_pending) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(led_cmd);            // byte 0 o 1
    uint8_t err = Wire.endTransmission();

    Serial.print("TX cmd=");
    Serial.print(led_cmd);
    Serial.print(" err=");
    Serial.println(err);            // 0 OK, 2 NACK addr

    led_cmd_pending = false;
  }
}

void shell() {
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd == "1") {
    led_cmd = 1;
    led_cmd_pending = true;
    Serial.println("-> LED ON");
  } else if (cmd == "0") {
    led_cmd = 0;
    led_cmd_pending = true;
    Serial.println("-> LED OFF");
  } else {
    Serial.println("ERROR: Use 1 o 0");
  }
}