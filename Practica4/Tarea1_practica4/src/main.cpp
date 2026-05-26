#include <ArduinoBLE.h>

// UUID: 50455045-0000-0000-0000-000000000000 

BLEService beaconService("4D617263-6F00-0000-0000-000000000000");    // Marco en ASCII hex: 4D 61 72 63 6F 00 00 00 00 00 00 00 00 00 00 00

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Error iniciando BLE");
    while (1);
  }


  BLE.setLocalName("Marco-Beacon");

  // Añadir el servicio con el UUID que contiene el nombre
  BLE.setAdvertisedService(beaconService);
  BLE.addService(beaconService);

  // Iniciar advertising
  BLE.advertise();

  Serial.println("Advertising BLE iniciado");
  Serial.println("UUID: 50455045-0000-0000-0000-000000000000");
}

void loop() {
  BLE.poll();
}