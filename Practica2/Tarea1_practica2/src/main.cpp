#include <Arduino.h>

#define LED_PIN LED_BUILTIN

bool led_state = false;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code( void * parameter);
void Task2code( void * parameter);

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  xTaskCreatePinnedToCore(
    Task1code, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */

  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 0, &Task2, 1); 


}

void loop() {
  Serial.println("-------------------------LOOP-----------------------------------");

  vTaskDelay(2000);
}


void Task1code( void * parameter) {       //TAREA LED
  for(;;) {
    Serial.println("-------------------------TASK1-----------------------------------");
    if(led_state == true){
      digitalWrite(LED_PIN, LOW);
      led_state = false;
    } else {
      digitalWrite(LED_PIN, HIGH);
      led_state = true;
    }
    
    vTaskDelay(200);
  }
}

void Task2code( void * parameter) {           //TAREA SERIAL "hola mundo"

  for(;;) {
    Serial.println("-------------------------TASK2-----------------------------------");
    Serial.println("Hola mundo");
    vTaskDelay(1000);
  }
}

