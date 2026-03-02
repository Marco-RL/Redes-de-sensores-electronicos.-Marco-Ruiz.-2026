#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define I2C_ADDR 0x08
#define LED_PIN LED_BUILTIN

volatile bool frame_ready = false;
volatile uint8_t rx_buffer[16];
volatile uint8_t rx_len = 0;

unsigned long led_until = 0;

static inline int16_t read_i16_le(uint8_t lo, uint8_t hi) {
  return (int16_t)(lo | (hi << 8));
}

void onReceive(int howMany) {
  uint8_t i = 0;
  while (Wire.available() && i < sizeof(rx_buffer)) {
    rx_buffer[i++] = (uint8_t)Wire.read();
  }
  rx_len = i;
  frame_ready = true;

  led_until = millis() + 1000; // LED 1s
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin(I2C_ADDR);         // SLAVE real
  Wire.onReceive(onReceive);

  Serial.println("ESP32-S3 SLAVE listo");
}

void loop() {
  delay(1); // evita que el loop corra a toda velocidad (no bloqueante)
  // LED no bloqueante
  digitalWrite(LED_PIN, (millis() < led_until) ? HIGH : LOW);

  if (frame_ready) {
    noInterrupts();
    uint8_t buf[16];
    uint8_t len = rx_len;
    for (uint8_t i = 0; i < len; i++) buf[i] = rx_buffer[i];
    frame_ready = false;
    interrupts();

    // Esperamos len=8: 0xA5, idx, Ax,Ay,Az (int16)
    if (len == 8 && buf[0] == 0xA5) {
      uint8_t idx = buf[1];

      int16_t ax_i = read_i16_le(buf[2], buf[3]);
      int16_t ay_i = read_i16_le(buf[4], buf[5]);
      int16_t az_i = read_i16_le(buf[6], buf[7]);

      float ax = ax_i / 1000.0f;
      float ay = ay_i / 1000.0f;
      float az = az_i / 1000.0f;

      Serial.print("S"); Serial.print(idx); Serial.print(" -> ");
      Serial.print(ax, 3); Serial.print("; ");
      Serial.print(ay, 3); Serial.print("; ");
      Serial.println(az, 3);
    } else {
      Serial.print("Trama invalida. len=");
      Serial.println(len);
    }
  }
}