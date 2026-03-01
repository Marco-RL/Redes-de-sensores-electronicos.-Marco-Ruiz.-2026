#include <Arduino.h>
#include <Wire.h>


#define I2C_SDA 5
#define I2C_SCL 6
#define I2C_ADDR 0x08

volatile uint8_t led_cmd = 0;
volatile bool led_cmd_pending = false;

void shell();

void setup() {
  Serial.begin(115200);

  // ESP32-S3 como MASTER I2C (hay que indicar pines)
  //Wire.begin(I2C_SDA, I2C_SCL);

  bool ok = Wire.begin(I2C_SDA, I2C_SCL);
  if (ok) {
    Serial.println("I2C master iniciado correctamente");
  } else {
    Serial.println("ERROR iniciando I2C Master");
  }

  Wire.setClock(100000);   // 100 kHz para empezar (recomendado)

  Serial.println("ESP32-S3 I2C MASTER listo");
}

void loop() {
  if (Serial.available()) shell();

  if (led_cmd_pending) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(led_cmd);
    uint8_t err = Wire.endTransmission();

    Serial.print("TX cmd=");
    Serial.print(led_cmd);
    Serial.print("  err=");
    Serial.println(err); // 0 = OK

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