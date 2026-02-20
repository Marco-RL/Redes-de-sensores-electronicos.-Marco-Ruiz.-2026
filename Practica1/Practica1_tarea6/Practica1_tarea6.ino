//fuentes: https://github.com/arduino-libraries/Arduino_LSM9DS1/tree/master

#include <Arduino_LSM9DS1.h>
#include <Arduino.h>
#include "BBTimer.hpp"
#include <mbed.h>         //documentacion de la libreria https://os.mbed.com/docs/mbed-os/v6.15/apis/pwmout.html


BBTimer my_t0(BB_TIMER0);   // 100 ms
BBTimer my_t1(BB_TIMER1);   // 1 s

bool flag_LSM9DS1_read = false;
bool flag_print = false;

int sampleSeconds_us = 100000;    // 100 ms
int printPeriod_us  = 1000000;    // 1 s
 
void t0Callback(){
	flag_LSM9DS1_read = true;   //Activamos una flag para realizar la lectura del sensor y muestra en panatalla. Tambien se usara para imprimir si es necesario.
}
void t1Callback(){
  flag_print = true;
}


void setup() {
  Serial.begin(115200);

  my_t0.setupTimer(sampleSeconds_us, t0Callback);
	my_t0.timerStart();

  my_t1.setupTimer(printPeriod_us, t1Callback);
  my_t1.timerStart(); 

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
  }

}
float Acel_x[10], Acel_y[10], Acel_z[10];
float Gyro_x[10], Gyro_y[10], Gyro_z[10];
float Magne_x[10], Magne_y[10], Magne_z[10];


int sampleIndex = 0;

void loop() {

  // ---------- Adquisición cada 100 ms ----------
  if(flag_LSM9DS1_read){

    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(
        Acel_x[sampleIndex],
        Acel_y[sampleIndex],
        Acel_z[sampleIndex]
      );
    }

    if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(
        Gyro_x[sampleIndex],
        Gyro_y[sampleIndex],
        Gyro_z[sampleIndex]
      );
    }

    if (IMU.magneticFieldAvailable()) {
      IMU.readMagneticField(
        Magne_x[sampleIndex],
        Magne_y[sampleIndex],
        Magne_z[sampleIndex]
      );
    }

    sampleIndex++;

    if(sampleIndex >= 10){
      sampleIndex = 0;
    }

    flag_LSM9DS1_read = false;
  }

  // ---------- Envío cada 1 segundo ----------
  if(flag_print){

    for(int i = 0; i < 10; i++){

      Serial.print(Acel_x[i]); Serial.print(";");
      Serial.print(Acel_y[i]); Serial.print(";");
      Serial.print(Acel_z[i]); Serial.print(";");

      Serial.print(Gyro_x[i]); Serial.print(";");
      Serial.print(Gyro_y[i]); Serial.print(";");
      Serial.print(Gyro_z[i]); Serial.print(";");

      Serial.print(Magne_x[i]); Serial.print(";");
      Serial.print(Magne_y[i]); Serial.print(";");
      Serial.println(Magne_z[i]);

    }

    flag_print = false;
  }
}