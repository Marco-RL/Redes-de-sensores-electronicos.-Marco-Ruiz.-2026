// mpaterial de apoyo:   https://cpp4arduino.com/2018/10/23/what-is-string-interning.html    https://www.arduino.cc/reference/en/language/variables/data-types/string/https://www.arduino.cc/reference/en/language/variables/data-types/string/

// Codigo basado en     https://forum.arduino.cc/t/five-hardware-timers-example/905798      

#include <Arduino.h>
#include "BBTimer.hpp"
#include <mbed.h>         //documentacion de la libreria https://os.mbed.com/docs/mbed-os/v6.15/apis/pwmout.html      

BBTimer my_t0(BB_TIMER0);

bool flag_ADC_read = false;

static mbed::PwmOut pwm(digitalPinToPinName(D9));

int sampleSeconds = 1000000;
 
void t0Callback(){
	flag_ADC_read = true;   //Activamos una flag para realizar la lectura del sensor y muestra en panatalla. Tambien se usara para imprimir si es necesario.
}

void setup() {
  Serial.begin(115200);
	my_t0.setupTimer(sampleSeconds, t0Callback);
	my_t0.timerStart();
  pwm.period_us(200);
  pwm.write(0.0);    //colocamos un duty iniial de 0


}

uint32_t raw_ADC;
uint32_t voltaje_ADC;
bool imprimirADC = false;

void loop() {

  if (Serial.available() != 0) {
    shell();
  }
  if(flag_ADC_read == true || imprimirADC == true){
    raw_ADC= analogRead(A0);
    voltaje_ADC = raw_ADC * 3300/1024;

    //Serial.print(">Voltaje ADC:");      //sistema de prints para la extension de visual studio de Teleplot
    Serial.println(voltaje_ADC);
    
    flag_ADC_read = false;        // bajamos la flag
    imprimirADC = false;
  }
}

void shell(){
  String cmd;
    cmd = Serial.readStringUntil('\n');
    cmd.trim();                       // con esto eliminamos todos los espacios en blanco del string guardado
    cmd.toUpperCase();                // para tener todo en el mismo formato, en mayusculas.
    
    Serial.println(cmd);

    if(cmd=="COMANDOS"){
        Serial.println("-----------------------------");
        Serial.print("\t");
        Serial.println("Seccion de comandos");
        Serial.println("-----------------------------");

        Serial.println(F("Envíe la lectura del ADC actual: ADC"));                  // con la f conseguimos guardarlo en la flash para asi ahorrar ram
        Serial.println(F("Envíe la lectura del ADC cada x segundos. Si x=0 deja de mandar datos: ADC(x)"));
        Serial.println(F("Envie el duty cycle del módulo PWM con números del 0 al 9: PWM(x)"));


    }else if(cmd=="ADC"){//------------------------------------------------------
    imprimirADC = true;

    }else if(cmd.startsWith("ADC(") && cmd.endsWith(")")){//------------------------------------------------------
      String sampleTimeSeg = cmd.substring(4, cmd.length()-1);
      int seconds = sampleTimeSeg.toInt();
      
      if(seconds == 0){// seguimos leyendo pero no imprimimos nada
        Serial.println(F("->Ya no se mandaran mas dartos de manera periodica."));
        my_t0.timerStop();

      }else if(seconds < 0){
        Serial.println(F("->ERROR: Tiempo menor que 0."));

      }else if(seconds > 0){
        Serial.print(F("->Impresion de lectura del ADC cada "));
        Serial.print(seconds);
        Serial.println(F(" segundos. "));
        my_t0.updatePeriod(seconds * 1000000);  // segundos a microsegundos
        my_t0.timerStart(); 
      }

    }else if(cmd.startsWith("PWM(") && cmd.endsWith(")")){//------------------------------------------------------
      String dutyCycle = cmd.substring(4, cmd.length()-1);
      int intDutyCycle = dutyCycle.toInt();              
      if(intDutyCycle >= 0 && intDutyCycle <= 9){// REVISAR FLOATS AQUI Y AJUSTAR SI ES NECESARIO
        int dutyAjustado = intDutyCycle / 9;
        pwm.write(dutyAjustado);
        Serial.println("->PWM actualizado");
      }else {
        Serial.println("->Valor PWM invalido");
      }

     }else{//-----------------------------------------------------------------------------
      Serial.println("ERROR: Comando no valido");
    }
}



