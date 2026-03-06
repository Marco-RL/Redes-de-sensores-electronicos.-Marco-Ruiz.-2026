#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "Redmi Note 9S";
const char* password = "aaaaaaaa";

WiFiClient client;
WiFiServer server(5000);       // El ESP32 queda escuchando en el puerto 5000

String rxMessage = "";

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

  server.begin();  // Iniciamos el servidor
  Serial.println("Server started, waiting for clients...");
  
}

void loop() {

  if(!client || !client.connected()) {
    client = server.available();  // Esperamos a que un cliente se conecte
    if(client) {
      Serial.println("Client connected!");
    }
  }

  // Leer datos recibidos desde el móvil
  
  if (client && client.available()) {
    rxMessage = client.readStringUntil('\n');
    rxMessage.trim();  // Eliminar espacios en blanco

    if (rxMessage.length() > 0) {
      Serial.print("Received: ");
      Serial.println(rxMessage);
    }
  }

  // También puedes escribir desde el monitor serie al móvil
  if (Serial.available()) {
    String txMessage = Serial.readStringUntil('\n');
    txMessage.trim();

    if (txMessage.length() > 0) {
      client.print("ESP32: ");
      client.println(txMessage);
    }
  }

  // Detectar desconexión
  if (client && !client.connected()) {
    Serial.println("Cliente desconectado");
    client.stop();
  }
}