#include <ArduinoJson.h>

#define LED_PIN 7  // Usamos el pin digital 7 para el relé o LED

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("🔌 Arduino listo para recibir comandos Serial...");
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
          digitalWrite(LED_PIN, ledState ? LOW : HIGH);
          Serial.print("LED ");
          Serial.println(ledState ? "APAGADO ❌" : "ENCENDIDO ✅");
        } else {
          Serial.println("⚠️ Clave 'led' no encontrada.");
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
