#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

//http://192.168.43.167/
/*CONECTAR EL PC A LA MISMA RED */

const char* ssid     = "Redmi Note 9S";
const char* password = "aaaaaaaa";

// Servidor HTTP en puerto 80
AsyncWebServer server(80);

// Tiempo de inicio (para calcular hora transcurrida)
unsigned long startTime = 0;

// Función que reemplaza %PLACEHOLDER% en el HTML
String processor(const String& var) {
  if (var == "TIME") {
    unsigned long elapsed = (millis() - startTime) / 1000;
    unsigned long hh = elapsed / 3600;
    unsigned long mm = (elapsed % 3600) / 60;
    unsigned long ss = elapsed % 60;

    char buf[9];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hh, mm, ss);
    return String(buf);
  }
  return String();
}

void setup() {
  Serial.begin(115200);

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }
  Serial.println("SPIFFS montado correctamente");

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  startTime = millis();

  // --- Rutas del servidor ---

  // Página principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // CSS
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Endpoint para obtener la hora actual (llamado por JavaScript cada segundo)
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    unsigned long elapsed = (millis() - startTime) / 1000;
    unsigned long hh = elapsed / 3600;
    unsigned long mm = (elapsed % 3600) / 60;
    unsigned long ss = elapsed % 60;

    char buf[9];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hh, mm, ss);
    request->send(200, "text/plain", String(buf));
  });

  // Endpoint para resetear la hora
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    startTime = millis();
    Serial.println("Hora reseteada a 00:00:00");
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  // Nada aquí, el servidor es asíncrono
}