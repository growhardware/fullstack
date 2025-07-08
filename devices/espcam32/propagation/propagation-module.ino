// 🪴 GrowHardware Propagation Module - Extended MQTT Firmware (Advanced)
// + Soporta: horario encendido/apagado, phases, captura de foto tras cambio luz, publicación en /status, HTTP directo

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DHT.h"
#include <BH1750.h>
#include "esp_camera.h"
#include <time.h>

// WiFi y MQTT
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";
const char* mqtt_server = "BROKER_IP";
const char* backend_url = "http://TU_BACKEND_URL/device/post-photo";
const char* device_id = "propagation_001";

WiFiClient espClient;
PubSubClient client(espClient);

// Sensores
#define DHTPIN 13
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

// GPIOs de salida
#define LIGHT_RELAY_GPIO 2
#define HUMIDITY_RELAY_GPIO 14

// Estado de plan
bool light_on = false;
bool humidity_on = false;
float target_humidity = 70.0;
String phase = "veg";
int light_on_hour = 6;
int light_off_hour = 0; // 0 = siempre encendida

// Temporización
unsigned long lastMsg = 0;
const long interval = 10000; // 10 segundos
unsigned long lastPhoto = 0;
const long photoInterval = 60000 * 15; // cada 15 minutos

void publishStatus(float temp, float hum, float lux) {
  String payload = "{";
  payload += "\"device_id\":\"" + String(device_id) + "\",";
  payload += "\"temperature\":" + String(temp) + ",";
  payload += "\"humidity\":" + String(hum) + ",";
  payload += "\"lux\":" + String(lux) + ",";
  payload += "\"phase\":\"" + phase + "\"";
  payload += "}";
  client.publish(("gh-mqtt/" + String(device_id) + "/status").c_str(), payload.c_str());
}

void takeAndSendPhoto() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Fallo al capturar foto");
    return;
  }

  WiFiClient client;
  if (client.connect("TU_BACKEND_URL", 80)) {
    String req = String("POST ") + "/device/post-photo?device=" + device_id + " HTTP/1.1\r\n" +
                 "Host: TU_BACKEND_URL\r\n" +
                 "Content-Type: image/jpeg\r\n" +
                 "Content-Length: " + fb->len + "\r\n\r\n";

    client.print(req);
    client.write(fb->buf, fb->len);
    delay(100);
    client.stop();
    Serial.println("📸 Foto enviada por HTTP");
  } else {
    Serial.println("Error al conectar con backend HTTP");
  }
  esp_camera_fb_return(fb);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Mensaje MQTT: " + message);

  int planIndex = message.indexOf("\"plan\":");
  if (planIndex != -1) {
    String plan = message.substring(planIndex + 7);

    if (plan.indexOf("light_on":true) != -1) {
      light_on = true;
    } else if (plan.indexOf("light_on":false) != -1) {
      light_on = false;
    }

    if (plan.indexOf("humidity_on":true) != -1) {
      humidity_on = true;
    } else if (plan.indexOf("humidity_on":false) != -1) {
      humidity_on = false;
      digitalWrite(HUMIDITY_RELAY_GPIO, LOW);
    }

    int idx = plan.indexOf("target_humidity":);
    if (idx != -1) {
      String sub = plan.substring(idx + 17);
      target_humidity = sub.toFloat();
    }

    int onH = plan.indexOf("light_on_hour":);
    int offH = plan.indexOf("light_off_hour":);
    if (onH != -1 && offH != -1) {
      light_on_hour = plan.substring(onH + 16, plan.indexOf(",", onH)).toInt();
      light_off_hour = plan.substring(offH + 17).toInt();
    }

    int p = plan.indexOf("phase":);
    if (p != -1) {
      int q1 = plan.indexOf("\"", p + 7);
      int q2 = plan.indexOf("\"", q1 + 1);
      phase = plan.substring(q1 + 1, q2);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando MQTT...");
    if (client.connect(device_id)) {
      Serial.println("Conectado");
      client.subscribe(("gh-mqtt/" + String(device_id) + "/updates").c_str());
    } else {
      Serial.print("Fallo: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT_RELAY_GPIO, OUTPUT);
  pinMode(HUMIDITY_RELAY_GPIO, OUTPUT);
  digitalWrite(LIGHT_RELAY_GPIO, LOW);
  digitalWrite(HUMIDITY_RELAY_GPIO, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();
  Wire.begin();
  lightMeter.begin();

  configTime(0, 0, "pool.ntp.org"); // hora NTP
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;

  bool lightShouldBeOn = (light_off_hour > 0)
    ? (hour >= light_on_hour && hour < light_off_hour)
    : light_on;

  static bool prevLightState = false;
  digitalWrite(LIGHT_RELAY_GPIO, lightShouldBeOn);

  if (lightShouldBeOn != prevLightState) {
    takeAndSendPhoto();
    prevLightState = lightShouldBeOn;
  }

  unsigned long nowMillis = millis();
  if (nowMillis - lastMsg > interval) {
    lastMsg = nowMillis;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    float lux = lightMeter.readLightLevel();

    if (!isnan(temp) && !isnan(hum) && !isnan(lux)) {
      if (humidity_on && hum < target_humidity) {
        digitalWrite(HUMIDITY_RELAY_GPIO, HIGH);
      } else {
        digitalWrite(HUMIDITY_RELAY_GPIO, LOW);
      }
      publishStatus(temp, hum, lux);
    }
  }

  if (nowMillis - lastPhoto > photoInterval) {
    lastPhoto = nowMillis;
    takeAndSendPhoto();
  }
}
