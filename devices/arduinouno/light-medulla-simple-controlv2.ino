#include <ArduinoJson.h>

#define LED_PIN 7

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Apagar al inicio
  Serial.println("✅ Arduino listo para control de LED via Serial");
}

void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, input);

      if (!error) {
        if (doc.containsKey("led")) {
          bool ledState = doc["led"];
          digitalWrite(LED_PIN, ledState ? LOW : HIGH);  // LOW = encendido en relé normalmente abierto
          Serial.print("💡 LED ");
          Serial.println(ledState ? "ENCENDIDO ✅" : "APAGADO ❌");
        } else {
          Serial.println("⚠️ Clave 'led' no encontrada en JSON.");
        }
      } else {
        Serial.print("❌ Error de JSON: ");
        Serial.println(error.c_str());
      }

      input = "";  // Limpiar buffer
    } else {
      input += c;
    }
  }
}
