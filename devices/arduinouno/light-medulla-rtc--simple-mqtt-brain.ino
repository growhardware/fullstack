// ... necesitás un módulo de reloj en tiempo real (RTC), como el DS3231 o DS1307, ya que el Arduino UNO no tiene reloj interno con batería.
#include <ArduinoJson.h>
#include <Wire.h>
#include "RTClib.h"

#define LED_PIN 7

RTC_DS3231 rtc;

int onHour = 0;
int offHour = 0;
bool manualLedState = false;
bool autoMode = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!rtc.begin()) {
    Serial.println("❌ No se detectó el RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("⚠️ RTC sin hora, configurando hora actual...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Usa hora de compilación
  }

  Serial.println("🕒 Arduino con RTC listo!");
}

void loop() {
  handleSerial();
  handleLightBySchedule();
  delay(1000);
}

void handleSerial() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, input);

      if (!error) {
        if (doc.containsKey("led")) {
          manualLedState = doc["led"];
          autoMode = false;
          digitalWrite(LED_PIN, manualLedState ? LOW : HIGH);
          Serial.println(manualLedState ? "LED APAGADO ❌" : "LED ENCENDIDO ✅");
        }

        if (doc.containsKey("on_hour") && doc.containsKey("off_hour")) {
          onHour = doc["on_hour"];
          offHour = doc["off_hour"];
          autoMode = true;
          Serial.print("Programado encendido: ");
          Serial.print(onHour);
          Serial.print("h, apagado: ");
          Serial.print(offHour);
          Serial.println("h ✅");
        }
      } else {
        Serial.print("❌ Error JSON: ");
        Serial.println(error.c_str());
      }

      input = "";
    } else {
      input += c;
    }
  }
}

void handleLightBySchedule() {
  if (!autoMode) return;

  DateTime now = rtc.now();
  int currentHour = now.hour();

  bool lightShouldBeOn = (offHour > onHour)
    ? (currentHour >= onHour && currentHour < offHour)
    : (currentHour >= onHour || currentHour < offHour);  // Soporta horario cruzado

  digitalWrite(LED_PIN, lightShouldBeOn ? LOW : HIGH);
}
