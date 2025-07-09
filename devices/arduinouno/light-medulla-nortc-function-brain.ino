#include <ArduinoJson.h>

#define LED_PIN 7

int onHour = -1;
int onMinute = -1;
int offHour = -1;
int offMinute = -1;
int currentHour = -1;
int currentMinute = -1;
bool ledState = false;
bool hasManualControl = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("🕒 Arduino con control HH:MM ✅");
}

void loop() {
  handleSerial();
  checkSchedule();
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
        if (doc.containsKey("current_time")) {
          String t = doc["current_time"]; // formato "HH:MM"
          currentHour = t.substring(0, 2).toInt();
          currentMinute = t.substring(3, 5).toInt();
        }

        if (doc.containsKey("led")) {
          ledState = doc["led"];
          hasManualControl = true;
          digitalWrite(LED_PIN, ledState ? LOW : HIGH);
          Serial.println(ledState ? "LED APAGADO ❌ (manual)" : "LED ENCENDIDO ✅ (manual)");
        }

        if (doc.containsKey("on_time") && doc.containsKey("off_time")) {
          String on = doc["on_time"];   // formato "HH:MM"
          String off = doc["off_time"];
          onHour = on.substring(0, 2).toInt();
          onMinute = on.substring(3, 5).toInt();
          offHour = off.substring(0, 2).toInt();
          offMinute = off.substring(3, 5).toInt();
          hasManualControl = false;

          Serial.print("⏱️ Programado de ");
          Serial.print(on);
          Serial.print(" a ");
          Serial.println(off);
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

bool isBetween(int ch, int cm, int sh, int sm, int eh, int em) {
  if (sh < eh || (sh == eh && sm < em)) {
    return (ch > sh || (ch == sh && cm >= sm)) &&
           (ch < eh || (ch == eh && cm < em));
  } else {
    return (ch > sh || (ch == sh && cm >= sm)) ||
           (ch < eh || (ch == eh && cm < em));
  }
}

void checkSchedule() {
  if (hasManualControl || currentHour < 0 || onHour < 0) return;

  bool shouldBeOn = isBetween(currentHour, currentMinute, onHour, onMinute, offHour, offMinute);
  digitalWrite(LED_PIN, shouldBeOn ? LOW : HIGH);
}
