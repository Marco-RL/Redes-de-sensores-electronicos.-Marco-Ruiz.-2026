#include <Arduino.h>
#include <WiFi.h>

String recepcionData();
void envioPeriodico(int interval);
void envioData();
void getFakeAccelerometerData(float &ax, float &ay, float &az);

const char* ssid = "Redmi Note 9S";
const char* password = "aaaaaaaa";

WiFiClient client;
WiFiServer server(5000);

unsigned long lastSendTime = 0;

String command = "";
bool sendingData = false;

void setup() {
  Serial.begin(115200);
  randomSeed(micros());   //asi no tenemos la misma secuencia de números aleatorios cada vez que reiniciamos el ESP32

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Server started, waiting for clients...");
}

void loop() {
  if (!client || !client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Client connected!");
    }
  }
  
  envioData();

  command = recepcionData();
  command.toLowerCase();

  if (command == "start") {
    Serial.println("Iniciando envío periódico de datos fake del acelerómetro...");
    sendingData = true;
  } 
  else if (command == "stop") {
    Serial.println("Deteniendo envío periódico de datos fake del acelerómetro...");
    sendingData = false;
  }

  if (sendingData == true) {
    envioPeriodico(3000);
  }

  if (client && !client.connected()) {
    Serial.println("Cliente desconectado");
    client.stop();
    sendingData = false;
  }
}

String recepcionData() {
  String rxMessage = "";
  if (client && client.available()) {
    rxMessage = client.readStringUntil('\n');
    rxMessage.trim();

    if (rxMessage.length() > 0) {
      Serial.print("Received: ");
      Serial.println(rxMessage);
    }
  }
  return rxMessage;
}

void envioData() {
  if (Serial.available()) {
    String txMessage = Serial.readStringUntil('\n');
    txMessage.trim();

    if (txMessage.length() > 0 && client && client.connected()) {
      client.print("ESP32: ");
      client.println(txMessage);
    }
  }
}

void envioPeriodico(int interval) {
  if (client && client.connected()) {
    if (millis() - lastSendTime >= interval) {
      lastSendTime = millis();

      float ax, ay, az;
      getFakeAccelerometerData(ax, ay, az);

      client.print(ax, 2);
      client.print(";");
      client.print(ay, 2);
      client.print(";");
      client.println(az, 2);

      Serial.print("Datos enviados: ");
      Serial.print(ax, 2);
      Serial.print(";");
      Serial.print(ay, 2);
      Serial.print(";");
      Serial.println(az, 2);
    }
  }
}

void getFakeAccelerometerData(float &ax, float &ay, float &az) {
  ax = random(-500, 501) / 100.0;
  ay = random(-500, 501) / 100.0;
  az = random(-500, 501) / 100.0;
}