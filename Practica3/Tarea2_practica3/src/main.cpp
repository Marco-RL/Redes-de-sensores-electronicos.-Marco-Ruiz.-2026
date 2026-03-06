#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

const char* ssid = "Redmi Note 9S";
const char* password = "aaaaaaaa";

// NTP server setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;    // Adjust this for your timezone
const int daylightOffset_sec = 3600;  // Adjust if DST is in effect


// Function that prints formatted date and time
void printDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  char formattedTime[80];  // Buffer to store the formatted string
  strftime(formattedTime, sizeof(formattedTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println(formattedTime);
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nConnected to WiFi!");

  // Configure NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("NTP time configured.");
}

void loop() {
  // Print formatted date and time
  printDateTime();
  delay(1000);
}

