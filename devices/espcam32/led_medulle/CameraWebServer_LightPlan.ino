#include "esp_camera.h"
#include <WiFi.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <PubSubClient.h>
#include <time.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================

const char* ssid = "moto e20 5713";
const char* password = "msn23hu6mqekip6";
const char* mqtt_server = "192.168.105.13";
const int mqtt_port = 1883;
const char* device_id = "683f6bd17203ac3d3aeab2f5";
const char* backend_url = "http://tu-backend.com/api/upload";

WiFiClient espClient;
PubSubClient client(espClient);

const int relayPin = 12;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

// DynamicJsonDocument plan(8192);
DynamicJsonDocument plan(24576); // 24KB

bool plan_received = false;
bool last_light_state = false;

String jwt_token;
String lastLog = "";

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

// #if defined(CAMERA_MODEL_ESP_EYE)
#if defined(CAMERA_MODEL_AI_THINKER)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  // setup_wifi();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  // initCamera();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("[Setup] Inicialización completa.");

// apago para ahorrar recursos
  // startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

}

// void callback(char* topic, byte* payload, unsigned int length) {
//   if (String(topic) == device_id) {
//     DeserializationError error = deserializeJson(plan, payload, length);
//     if (!error) {
//       plan_received = true;
//       Serial.println("\n[MQTT] Plan recibido correctamente:");
//       serializeJsonPretty(plan, Serial);
//     } else {
//       Serial.println("[MQTT] Error parseando el plan JSON.");
//     }
//   }
// }
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] Mensaje recibido en topic: ");
  Serial.println(topic);
  Serial.print("[MQTT] Payload: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (String(topic) == device_id) {
    DeserializationError error = deserializeJson(plan, payload, length);
    if (!error) {
      plan_received = true;
      Serial.println("\n[MQTT] Plan recibido correctamente:");
      serializeJsonPretty(plan, Serial);
      printLocalTime();
    } else {
      Serial.print("[MQTT] Error parseando el plan JSON: ");
      Serial.println(error.c_str());
    }
  }
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected ");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(device_id)) {
      client.subscribe(device_id);
      Serial.println("[MQTT] Suscrito al topic: " + String(device_id));
    } else {
      Serial.print("[MQTT] Conexión fallida, rc=");
      Serial.print(client.state());
      Serial.println(". Reintentando en 5s...");
      delay(5000);
    }
  }
}

bool isLightTime() {
  if (!plan_received) return false;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return false;

  char currentDate[11];
  strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", &timeinfo);
  char currentTime[6];
  strftime(currentTime, sizeof(currentTime), "%H:%M", &timeinfo);
  String now(currentTime);

  JsonArray phases = plan["plan"]["phases"];
  for (JsonObject phase : phases) {
    String type = phase["type"];
    if (type == "custom") {
      JsonArray schedule = phase["light_schedule"];
      for (JsonObject day : schedule) {
        if (String(day["date"]) == currentDate) {
          String on = day["light_on"];
          String off = day["light_off"];
          return (now >= on && now < off);
        }
      }
    } else {
      String start = phase["start_date"];
      if (String(currentDate) >= start) {
        String on = phase["light_on"];
        String off = phase["light_off"];
        return (now >= on && now < off);
      }
    }
  }
  return false;
}

void captureAndSendImage() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[Cam] Error capturando imagen.");
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.begin(client, backend_url);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);

  if (httpResponseCode > 0) {
    Serial.printf("[HTTP] Imagen enviada. Código: %d\n", httpResponseCode);
  } else {
    Serial.printf("[HTTP] Fallo al enviar imagen. Código: %d\n", httpResponseCode);
  }

  http.end();
  esp_camera_fb_return(fb);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  if (plan_received) {
    bool lightShouldBeOn = isLightTime();
    if (lightShouldBeOn != last_light_state) {
      digitalWrite(relayPin, lightShouldBeOn ? HIGH : LOW);
      Serial.print("[Relé] Luz ");
      Serial.println(lightShouldBeOn ? "ENCENDIDA" : "APAGADA");
      captureAndSendImage();
      last_light_state = lightShouldBeOn;
    }
  }
  delay(10000);
}



void logEvent(String msg) {
  lastLog = "[" + String(millis()) + "] " + msg;
  Serial.println(lastLog);
  // if (webSocket.isConnected()) {
  //   StaticJsonDocument<256> data;
  //   data["type"] = "log_event";
  //   data["payload"]["message"] = lastLog;
  //   String json;
  //   serializeJson(data, json);
  //   webSocket.sendTXT(json);
  // }
}

void printLocalTime(){
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("[Hora Local] ");
    Serial.println(timeString);
  } else {
    Serial.println("[Error] No se pudo obtener la hora local");
  }
}
