
// Firmware - ESP32-CAM (Propagation Module) [DEBUG MODE CORREGIDO]

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <esp_camera.h>
#include <FS.h>
#include <SD_MMC.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WebServer.h>

const char* ssid = "wifi-net";
const char* password = "wifi-password";
const char* login_path = "/api/v1/entrance/login";
const char* api_host = "http://host.docker.internal:3000";
const char* socket_host = "host.docker.internal";
const int socket_port = 3000;
const char* user_email = "hola@hola.io";
const char* user_password = "hola";

#define DHTPIN 13
#define DHTTYPE DHT22
#define LIGHT_SENSOR_PIN 2
#define RELAY_LIGHT 12
#define RELAY_HUMID 14
#define RELAY_WATER 15

WebSocketsClient webSocket;
WebServer server(80);
String jwt_token;
String lastLog = "";

// IP fija
IPAddress local_IP(192, 168, 154, 106);
IPAddress gateway(192, 168, 154, 1);
IPAddress subnet(255, 255, 255, 0);

// Estado
unsigned long start_time = 0;
int light_on = 18;
int light_off = 6;
float temp_threshold = 24.0;
float hum_min = 80;
float hum_max = 95;
bool water = false;
bool watered = false;

DHT dht(DHTPIN, DHTTYPE);

void logEvent(String msg) {
  lastLog = "[" + String(millis()) + "] " + msg;
  Serial.println(lastLog);
  if (webSocket.isConnected()) {
    StaticJsonDocument<256> data;
    data["type"] = "log_event";
    data["payload"]["message"] = lastLog;
    String json;
    serializeJson(data, json);
    webSocket.sendTXT(json);
  }
}

void handleLog() {
  server.send(200, "text/plain", lastLog);
}

void connectToWiFi() {
  WiFi.disconnect(true);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    logEvent("Conectado al WiFi. IP: " + WiFi.localIP().toString());
    digitalWrite(RELAY_LIGHT, HIGH);
  } else {
    logEvent("❌ Error conectando al WiFi. Modo AP activado.");
    WiFi.softAP("GrowHardware-Setup", "12345678");
  }
}

void loginUser() {
  HTTPClient http;
  http.begin(String(api_host) + login_path);
  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<200> doc;
  doc["emailAddress"] = user_email;
  doc["password"] = user_password;
  String requestBody;
  serializeJson(doc, requestBody);
  int code = http.POST(requestBody);
  if (code == 200) {
    String payload = http.getString();
    StaticJsonDocument<512> res;
    deserializeJson(res, payload);
    jwt_token = res["token"].as<String>();
    logEvent("✅ Login OK");
  } else {
    logEvent("❌ Login fallido");
  }
  http.end();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<512> msg;
    deserializeJson(msg, payload);
    if (msg["type"] == "plan:update") {
      JsonObject p = msg["payload"];
      light_on = p["light_on_hours"] | 18;
      light_off = p["light_off_hours"] | 6;
      start_time = p["start_time"] | 0;
      temp_threshold = p["temp_threshold"] | 24.0;
      hum_min = p["humidity_clone_min"] | 80;
      hum_max = p["humidity_clone_max"] | 95;
      water = p["water"];
      watered = false;
      logEvent("📦 Plan de cultivo actualizado");
    }
  }
}

void setupWebSocket() {
  String socketUrl = "/?token=" + jwt_token;
  webSocket.begin(socket_host, socket_port, socketUrl);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(RELAY_HUMID, OUTPUT);
  pinMode(RELAY_WATER, OUTPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  dht.begin();
  connectToWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    loginUser();
    if (jwt_token.length() > 10) setupWebSocket();
  }
  server.on("/log", handleLog);
  server.begin();
  logEvent("✅ Setup completo");
}

void loop() {
  webSocket.loop();
  server.handleClient();

  static unsigned long lastRead = 0;
  if (millis() - lastRead > 10000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      unsigned long now = millis() / 1000;
      unsigned long elapsed = now - start_time;
      unsigned long cycle = light_on + light_off;
      bool lightShouldBeOn = (elapsed % cycle) < light_on;
      digitalWrite(RELAY_LIGHT, lightShouldBeOn ? HIGH : LOW);
      if (h < hum_min) digitalWrite(RELAY_HUMID, HIGH);
      else if (h > hum_max) digitalWrite(RELAY_HUMID, LOW);
      if (water && !watered) {
        digitalWrite(RELAY_WATER, HIGH);
        delay(3000);
        digitalWrite(RELAY_WATER, LOW);
        watered = true;
      }
      StaticJsonDocument<256> data;
      data["type"] = "sensor_data";
      data["payload"]["temp"] = t;
      data["payload"]["hum"] = h;
      data["payload"]["light"] = lightShouldBeOn;
      data["payload"]["lux"] = digitalRead(LIGHT_SENSOR_PIN);
      String json;
      serializeJson(data, json);
      webSocket.sendTXT(json);
    }
    lastRead = millis();
  }
}
