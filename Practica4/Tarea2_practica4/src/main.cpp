#include <ArduinoBLE.h>

// Característica de escritura: 3 bytes [R, G, B] siendo [FF, FF, FF] el blanco y [00, 00, 00] el apagado

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLECharacteristic colorChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLEWriteWithoutResponse, 3);

BLEDescriptor colorDesc("2901", "Color RGB");  // descriptor de nombre

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  if (!BLE.begin()) {
    Serial.println("Error iniciando BLE");
    while (1);
  }

  BLE.setLocalName("Nano33-LED");
  BLE.setAdvertisedService(ledService);

  colorChar.addDescriptor(colorDesc);  // añadir descriptor a la característica

  ledService.addCharacteristic(colorChar);
  BLE.addService(ledService);

  uint8_t initVal[3] = {0, 0, 0};
  colorChar.writeValue(initVal, 3);

  BLE.advertise();
  Serial.println("BLE activo, esperando conexión...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Conectado: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (colorChar.written()) {
        uint8_t rgb[3];
        colorChar.readValue(rgb, 3);

        Serial.print("Color recibido -> R:");
        Serial.print(rgb[0]);
        Serial.print(" G:");
        Serial.print(rgb[1]);
        Serial.print(" B:");
        Serial.println(rgb[2]);

        analogWrite(LEDR, 255 - rgb[0]);
        analogWrite(LEDG, 255 - rgb[1]);
        analogWrite(LEDB, 255 - rgb[2]);
      }
    }

    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    Serial.println("Desconectado");
  }
}