#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>

const char* ssid = "Redmi Note 9S";
const char* password = "aaaaaaaa";

WiFiClient client;

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

  Serial.println("Checking connectivity with Google...");

  // CONEXION CON GOOGLE TCP

  if (client.connect("google.com", 80)) {
    Serial.println("Connection successful!");
    client.stop();
  } 
  else {
    Serial.println("Connection failed.");
  }

  //PING A GOOGLE

  Serial.println("Pinging google.com...");

  if (Ping.ping("google.com", 5)) {
    Serial.println("Ping successful!");
  } else {
    Serial.println("Ping failed.");
  }

  Serial.print("Average response time: ");
  Serial.print(Ping.averageTime());
  Serial.println(" ms");

}

void loop() {
}