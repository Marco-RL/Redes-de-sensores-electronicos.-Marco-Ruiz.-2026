#include "BluetoothSerial.h"

BluetoothSerial SerialBT;


void setup() {
  Serial.begin(115200);

  // Iniciar Bluetooth con el nombre que aparecerá en el móvil
  SerialBT.begin("ESP32-Chat");
  Serial.println("Bluetooth iniciado. Esperando conexión...");
}

void loop() {
  // Si llega un mensaje desde el móvil → mostrarlo por Serial Monitor
  if (SerialBT.available()) {
    String mensaje = SerialBT.readStringUntil('\n');
    Serial.print("Móvil → ESP32: ");
    Serial.println(mensaje);

    // Eco: reenviar el mensaje de vuelta al móvil
    SerialBT.print("ESP32 recibió: ");
    SerialBT.println(mensaje);
  }

  // Si escribimos algo en el Serial Monitor → enviarlo al móvil
  if (Serial.available()) {
    String mensaje = Serial.readStringUntil('\n');
    SerialBT.println(mensaje);
    Serial.print("ESP32 → Móvil: ");
    Serial.println(mensaje);
  }
}