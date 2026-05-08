#include <Arduino.h>
#include <EspMQTTClient.h>

// --- Credenciales WiFi y Adafruit IO ---
#define WIFI_SSID     "Redmi Note 9S"
#define WIFI_PASS     "TU_PASSWORD"
#define AIO_USERNAME  "TU_AIO_USERNAME"
#define AIO_KEY       "TU_AIO_KEY"

// Broker MQTT de Adafruit IO
#define AIO_SERVER    "io.adafruit.com"
#define AIO_PORT      1883

// Feed
#define FEED_PUBLISH  AIO_USERNAME "/feeds/tarea8-practica3"

EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASS,
  AIO_SERVER,
  AIO_USERNAME,   // usuario MQTT
  AIO_KEY,        // contraseña MQTT = AIO Key
  "ESP32_Client", // nombre del cliente
  AIO_PORT
);

unsigned long lastSendTime = 0;
const unsigned long INTERVAL = 10000;

float getFakeTemperature() {
  return 15.0 + random(0, 2500) / 100.0;
}

// Se llama automáticamente cuando la conexión está lista
void onConnectionEstablished() {
  Serial.println("Conectado a Adafruit IO via MQTT!");

  // --- PARTE 4: Suscripción al feed ---
  // Cuando escribas un valor desde el navegador, se recibirá aquí
  client.subscribe(FEED_PUBLISH, [](const String& payload) {
    Serial.print("Dato recibido desde Adafruit IO: ");
    Serial.println(payload);
  });
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());

  client.enableDebuggingMessages(); // muestra info de conexión
}

void loop() {
  client.loop(); // mantiene la conexión MQTT activa

  if (client.isConnected() && millis() - lastSendTime >= INTERVAL) {
    lastSendTime = millis();

    float temp = getFakeTemperature();
    String payload = String(temp, 2);

    Serial.print("Publicando temperatura: ");
    Serial.println(payload);

    // --- PARTE 3: Publicar dato en el feed ---
    client.publish(FEED_PUBLISH, payload);
  }
}