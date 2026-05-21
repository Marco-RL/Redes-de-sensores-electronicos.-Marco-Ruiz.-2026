#include <Arduino.h>
#include "esp_pm.h"
#include "esp_sleep.h"

#define LED_PIN LED_BUILTIN
#define NUM_SAMPLES 10

float ax_buf[NUM_SAMPLES], ay_buf[NUM_SAMPLES], az_buf[NUM_SAMPLES];
int sample_index = 0;

SemaphoreHandle_t bufferMutex;
TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void * parameter);
void Task2code(void * parameter);
void getFakeAccelerometerData(float &ax, float &ay, float &az);

void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Con light_sleep_enable=true el ESP32-S3 duerme solo en idle
  // No hace falta definir el hook manualmente
  esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 10,
    .light_sleep_enable = true
  };
  esp_pm_configure(&pm_config);

  bufferMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(Task1code, "Task1_Muestreo", 10000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(Task2code, "Task2_Envio",    10000, NULL, 1, &Task2, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}

// TAREA 1: Muestreo cada 100ms
void Task1code(void * parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    float ax, ay, az;
    getFakeAccelerometerData(ax, ay, az);

    if (xSemaphoreTake(bufferMutex, portMAX_DELAY)) {
      ax_buf[sample_index] = ax;
      ay_buf[sample_index] = ay;
      az_buf[sample_index] = az;
      sample_index = (sample_index + 1) % NUM_SAMPLES;
      xSemaphoreGive(bufferMutex);
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
  }
}

// TAREA 2: Envío UART cada 1s + LED 200ms
void Task2code(void * parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    float ax_copy[NUM_SAMPLES], ay_copy[NUM_SAMPLES], az_copy[NUM_SAMPLES];

    if (xSemaphoreTake(bufferMutex, portMAX_DELAY)) {
      memcpy(ax_copy, ax_buf, sizeof(ax_buf));
      memcpy(ay_copy, ay_buf, sizeof(ay_buf));
      memcpy(az_copy, az_buf, sizeof(az_buf));
      xSemaphoreGive(bufferMutex);
    }

    Serial.println("=== Acelerometro (ultimas 10 muestras) ===");
    for (int i = 0; i < NUM_SAMPLES; i++) {
      Serial.printf("  [%d] ax=%.2f  ay=%.2f  az=%.2f\n",
                    i, ax_copy[i], ay_copy[i], az_copy[i]);
    }

    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(200));
    digitalWrite(LED_PIN, LOW);

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}

void getFakeAccelerometerData(float &ax, float &ay, float &az) {
  ax = random(-500, 501) / 100.0;
  ay = random(-500, 501) / 100.0;
  az = random(-500, 501) / 100.0;
}