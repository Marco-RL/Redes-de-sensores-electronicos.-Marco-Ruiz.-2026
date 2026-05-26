#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1215");

// 12 bytes: 4 bytes por cada float (X, Y, Z)
BLECharacteristic accelChar("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead | BLENotify, 12);

BLEDescriptor accelDesc("2901", "Acelerometro XYZ");

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Iniciar IMU
  if (!IMU.begin()) {
    Serial.println("Error iniciando IMU");
    while (1);
  }
  Serial.print("Frecuencia acelerometro: ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");

  // Iniciar BLE
  if (!BLE.begin()) {
    Serial.println("Error iniciando BLE");
    while (1);
  }

  BLE.setLocalName("Nano33-IMU");
  BLE.setAdvertisedService(imuService);

  accelChar.addDescriptor(accelDesc);
  imuService.addCharacteristic(accelChar);
  BLE.addService(imuService);

  // Valor inicial a cero
  float initVal[3] = {0.0, 0.0, 0.0};
  accelChar.writeValue((byte*)initVal, 12);

  BLE.advertise();
  Serial.println("BLE activo, esperando conexión...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Conectado: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Solo enviamos si hay datos disponibles en el acelerómetro
      if (IMU.accelerationAvailable()) {
        float x, y, z;
        IMU.readAcceleration(x, y, z);

        // Empaquetar los 3 floats en un array de bytes
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