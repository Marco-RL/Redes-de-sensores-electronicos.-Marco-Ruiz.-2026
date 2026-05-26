#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1215");

// Característica de lectura/notificación del acelerómetro (tarea 3)
BLECharacteristic accelChar("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLENotify, 12);
BLEDescriptor accelDesc("2901", "Acelerometro XYZ");

// Característica de escritura para activar/desactivar (tarea 4)
BLECharacteristic controlChar("19B10002-E8F2-537E-4F6C-D104768A1215", BLEWrite, 1);
BLEDescriptor controlDesc("2901", "Control acelerometro");

bool accelEnabled = true;  // estado del acelerómetro

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Error iniciando IMU");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("Error iniciando BLE");
    while (1);
  }

  BLE.setLocalName("Nano33-IMU");
  BLE.setAdvertisedService(imuService);

  // Característica acelerómetro
  accelChar.addDescriptor(accelDesc);
  imuService.addCharacteristic(accelChar);

  // Característica control
  controlChar.addDescriptor(controlDesc);
  imuService.addCharacteristic(controlChar);

  BLE.addService(imuService);

  // Valores iniciales
  float initVal[3] = {0.0, 0.0, 0.0};
  accelChar.writeValue((byte*)initVal, 12);

  uint8_t initControl = 1;  // activado por defecto
  controlChar.writeValue(initControl);

  BLE.advertise();
  Serial.println("BLE activo, esperando conexión...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Conectado: ");
    Serial.println(central.address());

    while (central.connected()) {

      // Comprobar si se ha escrito en la característica de control
      if (controlChar.written()) {
        uint8_t cmd;
        controlChar.readValue(cmd);

        if (cmd == 1) {
          accelEnabled = true;
          Serial.println("Acelerometro ACTIVADO");
        } else if (cmd == 0) {
          accelEnabled = false;
          Serial.println("Acelerometro DESACTIVADO");
        }
      }

      // Solo leer y notificar si está activado
      if (accelEnabled && IMU.accelerationAvailable()) {
        float x, y, z;
        IMU.readAcceleration(x, y, z);

        float values[3] = {x, y, z};
        accelChar.writeValue((byte*)values, 12);

        Serial.print("X: "); Serial.print(x);
        Serial.print(" Y: "); Serial.print(y);
        Serial.print(" Z: "); Serial.println(z);
      }
    }

    Serial.println("Desconectado");
  }
}