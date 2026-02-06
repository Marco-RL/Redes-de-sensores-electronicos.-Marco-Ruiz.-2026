// Codigo basado en     https://forum.arduino.cc/t/five-hardware-timers-example/905798

#include <Arduino.h>
#include "BBTimer.hpp"

BBTimer my_t0(BB_TIMER0);

bool flag_10sec_read = false;
 
void t0Callback(){
	flag_10sec_read = true;   //Activamos una flag para realizar la lectura del sensor y muestra en panatalla. Tambien se usara para imprimir si es necesario.
}

void setup() {
  Serial.begin(115200);
	my_t0.setupTimer(10000000, t0Callback);
	my_t0.timerStart();

}

uint32_t raw_ADC;
uint32_t voltaje_ADC;

void loop() {
  if(flag_10sec_read == true){

    
    raw_ADC= analogRead(A0);
    voltaje_ADC = raw_ADC * 3300/1024;

    //Serial.print(">Voltaje ADC:");      //sistema de prints para la extension de visual studio de Teleplot
    Serial.println(voltaje_ADC);
    
    flag_10sec_read = false;        // bajamos la flag
  }
}




