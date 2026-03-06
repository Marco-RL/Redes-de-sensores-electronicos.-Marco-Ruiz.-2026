#include <Arduino_LSM9DS1.h>
#include <Arduino.h>
#include "BBTimer.hpp"
#include <mbed.h>
#include <Wire.h>

#define I2C_ADDR 0x08

BBTimer my_t0(BB_TIMER0);

bool flag_LSM9DS1_read = false;

int sampleSeconds_us = 200000;   // 200 ms

float Acel_x[5], Acel_y[5], Acel_z[5];
int sampleIndex = 0;

void t0Callback(){
  flag_LSM9DS1_read = true;
}

void setup() {
  Serial.begin(115200);

  Wire.begin();            // MASTER
  Wire.setClock(100000);

  my_t0.setupTimer(sampleSeconds_us, t0Callback);
  my_t0.timerStart();

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
  }

  Serial.println("Listo. Envia 'E' para mandar los datos por I2C.");
}

void loop() {

  // --------- Muestreo cada 200 ms ---------
  if(flag_LSM9DS1_read){

    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(
        Acel_x[sampleIndex],
        Acel_y[sampleIndex],
        Acel_z[sampleIndex]
      );

      sampleIndex++;
      if(sampleIndex >= 5){
        sampleIndex = 0;   // buffer circular
      }
    }

    flag_LSM9DS1_read = false;
  }

  // --------- Envío bajo demanda ---------
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'E' || c == 'e') {

      Serial.println("Enviando arrays por I2C...");

      Wire.beginTransmission(I2C_ADDR);

      Wire.write((uint8_t*)Acel_x, sizeof(Acel_x));
      Wire.write((uint8_t*)Acel_y, sizeof(Acel_y));
      Wire.write((uint8_t*)Acel_z, sizeof(Acel_z));

      uint8_t err = Wire.endTransmission();

      Serial.print("I2C err = ");
      Serial.println(err);
    }
  }
}