#include <Arduino.h>
#include <ArduinoJson.h>

unsigned long lastGenTime = 0;
const unsigned long INTERVAL = 10000; // 10 segundos

float getFakeTemperature() {
  return 15.0 + random(0, 2500) / 100.0;
}

void generarSenML() {
  unsigned long timestamp = millis() / 1000;
  float temperatura = getFakeTemperature();

  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();

  JsonObject record = array.add<JsonObject>();
  record["bn"] = "esp32/sensor/";
  record["bt"] = timestamp;
  record["n"]  = "temperature";
  record["u"]  = "Cel";
  record["v"]  = temperatura;
  record["t"]  = 0;

  String output;
  serializeJsonPretty(doc, output);

  Serial.println("--- SenML JSON generado ---");
  Serial.println(output);
  Serial.println("---------------------------");
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());

  lastGenTime = millis();
  generarSenML(); // Primera generación inmediata al arrancar
}

void loop() {
  if (millis() - lastGenTime >= INTERVAL) {
    lastGenTime = millis();
    generarSenML();
  }
}