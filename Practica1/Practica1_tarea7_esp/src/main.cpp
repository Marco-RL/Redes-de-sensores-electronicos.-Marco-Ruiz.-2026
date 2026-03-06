#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define I2C_ADDR 0x08
#define LED_PIN LED_BUILTIN

#define TOTAL_BYTES 60

volatile bool frame_ready = false;
volatile uint8_t rx_buffer[TOTAL_BYTES];
volatile uint8_t rx_len = 0;

unsigned long led_until = 0;

void onReceive(int howMany) {

  uint8_t i = 0;

  while (Wire.available() && i < TOTAL_BYTES) {
    rx_buffer[i++] = Wire.read();
  }

  rx_len = i;
  frame_ready = true;

  led_until = millis() + 1000;  // LED 1 segundo
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin(I2C_ADDR);      // SLAVE
  Wire.onReceive(onReceive);

  Serial.println("ESP32-S3 SLAVE listo");
}

void loop() {

  // -------- LED no bloqueante --------
  digitalWrite(LED_PIN, (millis() < led_until) ? HIGH : LOW);

  // -------- Procesar datos recibidos --------
  if (frame_ready) {

    noInterrupts();
    uint8_t local_buffer[TOTAL_BYTES];
    uint8_t len = rx_len;

    memcpy(local_buffer, (const void*)rx_buffer, TOTAL_BYTES);

    frame_ready = false;
    interrupts();

    if (len == TOTAL_BYTES) {

      float ax[5];
      float ay[5];
      float az[5];

      // Reconstrucción directa (memoria cruda)
      memcpy(ax, &local_buffer[0],  20);
      memcpy(ay, &local_buffer[20], 20);
      memcpy(az, &local_buffer[40], 20);

      Serial.println("Datos recibidos:");

      for (int i = 0; i < 5; i++) {
        Serial.print("Muestra ");
        Serial.print(i);
        Serial.print(" -> ");

        Serial.print(ax[i], 4); Serial.print(" ; ");
        Serial.print(ay[i], 4); Serial.print(" ; ");
        Serial.println(az[i], 4);
      }

      Serial.println("-------------------------");
    }
    else {
      Serial.print("Error: bytes recibidos = ");
      Serial.println(len);
    }
  }

  delay(1);   // IMPORTANTE en ESP32 slave
}