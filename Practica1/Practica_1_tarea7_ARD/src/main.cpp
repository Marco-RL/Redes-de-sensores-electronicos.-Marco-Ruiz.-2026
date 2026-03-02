#include <Arduino.h>
#include <Wire.h>
#include <Arduino_LSM9DS1.h>

#define I2C_ADDR       0x08
#define NUM_SAMPLES    5
#define SAMPLE_MS      200
#define SEND_GAP_MS    5     // separación entre frames I2C (no bloqueante)

// Frame: [0]=0xA5, [1]=idx, [2-7]=Ax,Ay,Az int16 (mg)
static inline int16_t pack_accel(float g) { return (int16_t)lroundf(g * 1000.0f); } // g->mg

enum State { IDLE, ACQUIRE, SEND };
State state = IDLE;

float ax_buf[NUM_SAMPLES], ay_buf[NUM_SAMPLES], az_buf[NUM_SAMPLES];
uint8_t sample_idx = 0;
uint8_t send_idx = 0;

unsigned long t_next_sample = 0;
unsigned long t_next_send = 0;

void start_acquire() {
  sample_idx = 0;
  send_idx = 0;
  t_next_sample = millis();       // primera muestra cuanto antes
  state = ACQUIRE;
  Serial.println(">> ACQUIRE: capturando 1s (5 muestras)...");
}

void setup() {
  Serial.begin(115200);

  Wire.begin();            // MASTER
  Wire.setClock(100000);

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU no inicializa");
    while (1) { }          // fallo fatal (esto sí puede ser bloqueante)
  }

  Serial.println("Nano MASTER listo. Envia 'S' para capturar+enviar.");
}

void loop() {
  // ---- Serial no bloqueante (lectura simple) ----
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'S' || c == 's') {
      if (state == IDLE) start_acquire();
      else Serial.println(">> Ocupado: espera a que termine.");
    }
  }

  // ---- Máquina de estados ----
  const unsigned long now = millis();

  if (state == ACQUIRE) {
    // Toca muestrear cada 200ms
    if (now >= t_next_sample) {
      // No esperamos con while: solo leemos si está disponible
      if (IMU.accelerationAvailable()) {
        float ax, ay, az;
        IMU.readAcceleration(ax, ay, az);

        ax_buf[sample_idx] = ax;
        ay_buf[sample_idx] = ay;
        az_buf[sample_idx] = az;

        sample_idx++;
        t_next_sample += SAMPLE_MS;  // programa siguiente tick

        if (sample_idx >= NUM_SAMPLES) {
          state = SEND;
          t_next_send = now;         // empezar a enviar ya
          Serial.println(">> SEND: enviando frames I2C...");
        }
      }
      // Si no está disponible todavía, no bloqueamos:
      // volverá a intentarlo en la próxima iteración de loop()
    }
  }

  else if (state == SEND) {
    // Enviar 1 frame cada SEND_GAP_MS (no bloqueante)
    if (send_idx < NUM_SAMPLES && now >= t_next_send) {
      int16_t ax_i = pack_accel(ax_buf[send_idx]);
      int16_t ay_i = pack_accel(ay_buf[send_idx]);
      int16_t az_i = pack_accel(az_buf[send_idx]);

      Wire.beginTransmission(I2C_ADDR);
      Wire.write(0xA5);
      Wire.write(send_idx);

      Wire.write((uint8_t)(ax_i & 0xFF));
      Wire.write((uint8_t)((ax_i >> 8) & 0xFF));
      Wire.write((uint8_t)(ay_i & 0xFF));
      Wire.write((uint8_t)((ay_i >> 8) & 0xFF));
      Wire.write((uint8_t)(az_i & 0xFF));
      Wire.write((uint8_t)((az_i >> 8) & 0xFF));

      uint8_t err = Wire.endTransmission();

      Serial.print("Frame ");
      Serial.print(send_idx);
      Serial.print(" err=");
      Serial.println(err);

      send_idx++;
      t_next_send = now + SEND_GAP_MS;
    }

    if (send_idx >= NUM_SAMPLES) {
      state = IDLE;
      Serial.println(">> DONE: listo para otro 'S'.");
    }
  }
}