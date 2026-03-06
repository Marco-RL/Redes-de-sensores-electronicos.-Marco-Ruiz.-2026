#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

String recepcionData();
void envioPeriodico(int interval);
void envioData();
String getTimeString();

const char* ssid = "Redmi Note 9S";
const char* password = "aaaaaaaa";

// NTP server setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

WiFiClient client;
WiFiServer server(5000);

unsigned long lastSendTime = 0;

String command = "";
bool sendingTime = false;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("NTP time configured.");

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

  if(command == "start"){
    Serial.println("Iniciando envío periódico de la hora...");
    sendingTime = true;
  } else if(command == "stop"){
    Serial.println("Deteniendo envío periódico de la hora...");
    sendingTime = false;
  }
  // Enviar la hora cada segundo al móvil
  if(sendingTime == true) {
    envioPeriodico(1000);
  }

  if (client && !client.connected()) {
    Serial.println("Cliente desconectado");
    client.stop();
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

void envioData(){
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

      String currentTime = getTimeString();

      client.print("Hora: ");
      client.println(currentTime);

      Serial.print("Hora enviada: ");
      Serial.println(currentTime);
    }
  }
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to obtain time";
  }

  char formattedTime[30];
  strftime(formattedTime, sizeof(formattedTime), "%H:%M:%S", &timeinfo);
  return String(formattedTime);
}