// light-medulla modo manual modo programado simulacion timer
// requiere simple brain con envio de hora cada un minuto al serial medula luz 
#include <Arduino.h>
#include <ArduinoJson.h>

const int RELAY_PIN = 7;

// Variables de configuración
String on_time = "06:00";
String off_time = "22:00";
bool manual_mode = false;
bool relay_state = false;

// Variables de tiempo
unsigned long last_sync = 0;
unsigned long last_minute = 0;
int current_hour = 0;
int current_minute = 0;
bool time_valid = false;

int timeToMinutes(String timeStr) {
    if (timeStr.length() != 5 || timeStr.charAt(2) != ':') return -1;
    
    int hours = timeStr.substring(0, 2).toInt();
    int minutes = timeStr.substring(3, 5).toInt();
    
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) return -1;
    return hours * 60 + minutes;
}

void updateRelay() {
    if (manual_mode || !time_valid) return;
    
    int on_mins = timeToMinutes(on_time);
    int off_mins = timeToMinutes(off_time);
    int current_mins = current_hour * 60 + current_minute;
    
    if (on_mins == -1 || off_mins == -1) return;
    
    bool new_state;
    if (on_mins < off_mins) {
        new_state = (current_mins >= on_mins && current_mins < off_mins);
    } else {
        new_state = (current_mins >= on_mins || current_mins < off_mins);
    }
    
    if (new_state != relay_state) {
        relay_state = new_state;
        digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
        Serial.print("Relay actualizado: ");
        Serial.println(relay_state ? "ON" : "OFF");
        
        // Enviar estado actual para monitoreo
        Serial.print("{\"status\":\"");
        Serial.print(relay_state ? "ON" : "OFF");
        Serial.print("\",\"time\":\"");
        Serial.print(current_hour < 10 ? "0" : "");
        Serial.print(current_hour);
        Serial.print(":");
        Serial.print(current_minute < 10 ? "0" : "");
        Serial.print(current_minute);
        Serial.println("\"}");
    }
}

void updateTime() {
    unsigned long now = millis();
    
    // Actualizar minutos cada 60,000 ms (1 minuto)
    if (now - last_minute >= 60000) {
        current_minute++;
        last_minute = now;
        
        if (current_minute >= 60) {
            current_minute = 0;
            current_hour = (current_hour + 1) % 24;
        }
        
        updateRelay();  // Verificar si necesita cambiar el relay
    }
    
    // Sincronizar con Node-RED cada 5 minutos (300,000 ms)
    if (now - last_sync >= 300000 && time_valid) {
        Serial.println("{\"sync_request\":true}");
        last_sync = now;
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Sistema iniciado. Esperando sincronización...");
}

void loop() {
    updateTime();  // Mantener el reloj interno actualizado
    
    // Procesar comandos entrantes
    if (Serial.available() > 0) {
        String jsonStr = Serial.readStringUntil('\n');
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, jsonStr);
        
        if (error) {
            Serial.print("JSON error: ");
            Serial.println(error.c_str());
            return;
        }
        
        // Sincronización de tiempo
        if (doc.containsKey("current_time")) {
            String new_time = doc["current_time"].as<String>();
            if (new_time.length() == 5 && new_time.charAt(2) == ':') {
                current_hour = new_time.substring(0, 2).toInt();
                current_minute = new_time.substring(3, 5).toInt();
                time_valid = true;
                last_minute = millis();  // Resetear contador
                last_sync = millis();
                
                Serial.print("Hora sincronizada: ");
                Serial.print(current_hour);
                Serial.print(":");
                Serial.println(current_minute);
            }
        }
        
        // Actualización de horarios
        if (doc.containsKey("on_time")) on_time = doc["on_time"].as<String>();
        if (doc.containsKey("off_time")) off_time = doc["off_time"].as<String>();
        
        // Comando manual
        if (doc.containsKey("led")) {
            manual_mode = true;
            bool new_state = doc["led"];
            if (new_state != relay_state) {
                relay_state = new_state;
                digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
                Serial.print("Modo manual: ");
                Serial.println(relay_state ? "ON" : "OFF");
            }
        } else if (doc.containsKey("on_time") || doc.containsKey("off_time")) {
            manual_mode = false;   // Vuelve a modo automático si se actualizan horarios
        }
        
        // Forzar verificación después de cambios
        updateRelay();
    }
}
// #include <Arduino.h>
// #include <ArduinoJson.h>

// const int RELAY_PIN = 7;

// // Variables globales para almacenar los horarios
// String current_time = "";
// String on_time = "";
// String off_time = "";
// bool manual_mode = true; // Comienza en modo manual
// bool relay_state = false;
// unsigned long last_check = 0;

// int timeToMinutes(String timeStr) {
//     if (timeStr.length() != 5 || timeStr.charAt(2) != ':') return -1;
    
//     int hours = timeStr.substring(0, 2).toInt();
//     int minutes = timeStr.substring(3, 5).toInt();
    
//     if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) return -1;
//     return hours * 60 + minutes;
// }

// void updateRelayState() {
//     if (manual_mode) return; // No actualizar en modo manual
    
//     int current_mins = timeToMinutes(current_time);
//     int on_mins = timeToMinutes(on_time);
//     int off_mins = timeToMinutes(off_time);
    
//     if (current_mins == -1 || on_mins == -1 || off_mins == -1) {
//         Serial.println("Error: Formato de tiempo inválido");
//         return;
//     }
    
//     bool new_state;
//     if (on_mins < off_mins) {
//         new_state = (current_mins >= on_mins && current_mins < off_mins);
//     } else {
//         new_state = (current_mins >= on_mins || current_mins < off_mins);
//     }
    
//     if (new_state != relay_state) {
//         relay_state = new_state;
//         digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
//         Serial.print("Relay actualizado automáticamente: ");
//         Serial.println(relay_state ? "ENCENDIDO" : "APAGADO");
//     }
// }

// void setup() {
//     Serial.begin(9600);
//     pinMode(RELAY_PIN, OUTPUT);
//     digitalWrite(RELAY_PIN, LOW);
//     Serial.println("Sistema iniciado. Esperando comandos...");
// }

// void loop() {
//     // Verificar el estado cada segundo
//     if (millis() - last_check >= 1000) {
//         if (!manual_mode && current_time != "") {
//             updateRelayState();
//         }
//         last_check = millis();
//     }
    
//     // Procesar comandos entrantes
//     if (Serial.available() > 0) {
//         String jsonStr = Serial.readStringUntil('\n');
//         DynamicJsonDocument doc(256);
//         DeserializationError error = deserializeJson(doc, jsonStr);
        
//         if (error) {
//             Serial.print("Error de JSON: ");
//             Serial.println(error.c_str());
//             return;
//         }
        
//         bool config_changed = false;
        
//         // Actualizar hora actual si viene en el mensaje
//         if (doc.containsKey("current_time")) {
//             String new_time = doc["current_time"].as<String>();
//             if (new_time != current_time) {
//                 current_time = new_time;
//                 config_changed = true;
//             }
//         }
        
//         // Actualizar horarios si vienen en el mensaje
//         if (doc.containsKey("on_time") && doc.containsKey("off_time")) {
//             String new_on = doc["on_time"].as<String>();
//             String new_off = doc["off_time"].as<String>();
            
//             if (new_on != on_time || new_off != off_time) {
//                 on_time = new_on;
//                 off_time = new_off;
//                 manual_mode = false;
//                 config_changed = true;
//                 Serial.println("Modo horario activado");
//             }
//         }
        
//         // Manejar comando manual
//         if (doc.containsKey("led")) {
//             bool new_state = doc["led"];
//             manual_mode = true;
//             if (new_state != relay_state) {
//                 relay_state = new_state;
//                 digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
//                 Serial.print("Modo manual: Relay ");
//                 Serial.println(relay_state ? "ENCENDIDO" : "APAGADO");
//             }
//         }
        
//         // Forzar actualización si hubo cambios
//         if (config_changed && !manual_mode) {
//             updateRelayState();
//         }
//     }
// }

// #include <Arduino.h>
// #include <ArduinoJson.h>  // Incluir la biblioteca necesaria

// const int RELAY_PIN = 7;  // Relay conectado al pin digital 7

// // Función para convertir string "HH:MM" a minutos totales
// int timeToMinutes(const char* timeStr) {
//     if (strlen(timeStr) != 5 || timeStr[2] != ':') return -1;
    
//     int hours = atoi(timeStr);
//     int minutes = atoi(timeStr + 3);
    
//     if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
//         return -1;
//     }
//     return hours * 60 + minutes;
// }

// void controlRelay(bool state) {
//     digitalWrite(RELAY_PIN, state ? HIGH : LOW);
// }

// void setup() {
//     Serial.begin(9600);
//     pinMode(RELAY_PIN, OUTPUT);
//     controlRelay(false);  // Estado inicial: apagado
// }

// void loop() {
//     if (Serial.available() > 0) {
//         String jsonStr = Serial.readStringUntil('\n');
        
//         // Parsear JSON
//         DynamicJsonDocument doc(256);
//         DeserializationError error = deserializeJson(doc, jsonStr);
        
//         if (error) {
//             Serial.print("Error de JSON: ");
//             Serial.println(error.c_str());
//             return;
//         }
        
//         // Extraer valores
//         const char* current_time = doc["current_time"] | "";
//         const char* on_time = doc["on_time"] | "";
//         const char* off_time = doc["off_time"] | "";
//         bool led = doc["led"] | false;
//         bool led_exists = doc.containsKey("led");
        
//         // 1. Prioridad: control por tiempo si existen todos los campos
//         if (strlen(current_time) > 0 && strlen(on_time) > 0 && strlen(off_time) > 0) {
//             int current_mins = timeToMinutes(current_time);
//             int on_mins = timeToMinutes(on_time);
//             int off_mins = timeToMinutes(off_time);
            
//             if (current_mins == -1 || on_mins == -1 || off_mins == -1) {
//                 Serial.println("Error: Formato de tiempo invalido");
//                 return;
//             }
            
//             // Calcular estado considerando cruce de medianoche
//             bool shouldTurnOn;
//             if (on_mins < off_mins) {
//                 shouldTurnOn = (current_mins >= on_mins && current_mins < off_mins);
//             } else {
//                 shouldTurnOn = (current_mins >= on_mins || current_mins < off_mins);
//             }
            
//             controlRelay(shouldTurnOn);
//             Serial.print("Modo horario: Relay ");
//             Serial.println(shouldTurnOn ? "ENCENDIDO" : "APAGADO");
//         }
//         // 2. Control directo si existe 'led' y los tiempos están vacíos
//         else if (led_exists && strlen(on_time) == 0 && strlen(off_time) == 0) {
//             controlRelay(led);
//             Serial.print("Control manual: Relay ");
//             Serial.println(led ? "ENCENDIDO" : "APAGADO");
//         }
//         else {
//             Serial.println("No se ejecutó: Datos insuficientes");
//         }
//     }
// }

// LED CONCEPT ////////////////
// #include <ArduinoJson.h>

// #define LED_PIN 7

// void setup() {
//   Serial.begin(9600);
//   pinMode(LED_PIN, OUTPUT);
//   digitalWrite(LED_PIN, LOW);
//   Serial.println("✅ Arduino listo para control de LED via Serial");
// }

// void loop() {
//   static String input = "";

//   while (Serial.available()) {
//     char c = Serial.read();

//     if (c == '\n') {
//       // 🔧 Workaround: limpiar cualquier basura antes del JSON
//       int jsonStart = input.indexOf('{');
//       if (jsonStart > 0) {
//         input = input.substring(jsonStart);  // Cortar desde '{'
//       }
//       Serial.print("📥 Recibido limpio: ");
//       Serial.println(input);  // Imprime todo el JSON

//       StaticJsonDocument<200> doc;
//       DeserializationError error = deserializeJson(doc, input);

//       if (error) {
//         Serial.print("❌ Error JSON: ");
//         Serial.println(error.c_str());
//       } else {
//         if (doc.containsKey("led")) {
//           bool led = doc["led"];
//           digitalWrite(LED_PIN, led ? LOW : HIGH);
//           Serial.println(led ? "💡 LED APAGADO ❌" : "💡 LED ENCENDIDO ✅");
//         } else {
//           Serial.print("📥 Recibido no encontrada: ");
//           Serial.println(input);
//           Serial.println("⚠️ Clave 'led' no encontrada en JSON.");
//         }
//       }

//       input = "";  // Limpiar buffer
//     } else {
//       input += c;
//     }
//   }
// }

// END LED CONCEPT ////////////////

// #include <ArduinoJson.h>

// #define LED_PIN 7

// int onHour = -1;
// int onMinute = -1;
// int offHour = -1;
// int offMinute = -1;
// int currentHour = -1;
// int currentMinute = -1;

// bool hasSchedule = false;
// bool hasCurrentTime = false;
// bool ledOverride = false;
// bool ledOverrideValue = false;

// void setup() {
//   Serial.begin(9600);
//   pinMode(LED_PIN, OUTPUT);
//   digitalWrite(LED_PIN, LOW);
//   Serial.println("✅ Arduino listo para control de LED via Serial con prioridad horaria");
// }

// void loop() {
//   handleSerial();
//   checkAndApplyLightLogic();
//   delay(1000);
// }

// void handleSerial() {
//   static String input = "";

//   while (Serial.available()) {
//     char c = Serial.read();
//     if (c == '\n') {
//       Serial.print("📥 RAW Recibido: ");
//       Serial.println(input);  // 👈🏽 VERIFICACIÓN DIRECTA

//       // Limpiar basura al inicio
//       int jsonStart = input.indexOf('{');
//       if (jsonStart > 0) input = input.substring(jsonStart);

//       StaticJsonDocument<256> doc;
//       DeserializationError error = deserializeJson(doc, input);

//       if (error) {
//         Serial.print("❌ Error al parsear JSON: ");
//         Serial.println(error.c_str());
//       } else {
//         // Actualizar hora actual
//         if (doc.containsKey("current_time")) {
//           String t = doc["current_time"];
//           currentHour = t.substring(0, 2).toInt();
//           currentMinute = t.substring(3, 5).toInt();
//           hasCurrentTime = true;
//           Serial.print("⏰ Hora actual recibida: ");
//           Serial.println(t);
//         }

//         // Actualizar programación horaria
//         if (doc.containsKey("on_time") && doc.containsKey("off_time")) {
//           String on = doc["on_time"];
//           String off = doc["off_time"];
//           onHour = on.substring(0, 2).toInt();
//           onMinute = on.substring(3, 5).toInt();
//           offHour = off.substring(0, 2).toInt();
//           offMinute = off.substring(3, 5).toInt();
//           hasSchedule = true;

//           Serial.print("📅 Programación: ");
//           Serial.print(on);
//           Serial.print(" → ");
//           Serial.println(off);
//         }

//         // Override manual
//         if (doc.containsKey("led")) {
//           ledOverride = true;
//           ledOverrideValue = doc["led"];
//           Serial.print("🎮 Control manual: ");
//           Serial.println(ledOverrideValue ? "APAGAR ❌" : "ENCENDER ✅");
//         }
//       }

//       input = "";
//     } else {
//       input += c;
//     }
//   }
// }

// bool isBetween(int ch, int cm, int sh, int sm, int eh, int em) {
//   if (sh < eh || (sh == eh && sm < em)) {
//     return (ch > sh || (ch == sh && cm >= sm)) &&
//            (ch < eh || (ch == eh && cm < em));
//   } else {
//     return (ch > sh || (ch == sh && cm >= sm)) ||
//            (ch < eh || (ch == eh && cm < em));
//   }
// }

// void checkAndApplyLightLogic() {
//   if (!hasCurrentTime) return;

//   bool shouldLightBeOn = false;

//   if (hasSchedule) {
//     shouldLightBeOn = isBetween(currentHour, currentMinute, onHour, onMinute, offHour, offMinute);
//     Serial.print("⏱️ En rango de horario? ");
//     Serial.println(shouldLightBeOn ? "✅ SÍ" : "❌ NO");
//   }

//   // Si está dentro del horario, encender luz
//   if (shouldLightBeOn) {
//     digitalWrite(LED_PIN, LOW);
//     Serial.println("💡 Encendido por horario ✅");
//   } else if (ledOverride) {
//     // Si hay override y no hay horario activo, aplicar
//     digitalWrite(LED_PIN, ledOverrideValue ? LOW : HIGH);
//     Serial.println(ledOverrideValue ? "💡 Apagado por override ❌" : "💡 Encendido por override ✅");
//   } else {
//     // Fuera de horario y sin override => apagar
//     digitalWrite(LED_PIN, HIGH);
//     Serial.println("💤 Apagado fuera de horario y sin override");
//   }
// }


// #include <ArduinoJson.h>

// #define LED_PIN 7

// int onHour = -1;
// int onMinute = -1;
// int offHour = -1;
// int offMinute = -1;
// int currentHour = -1;
// int currentMinute = -1;
// bool ledState = false;
// bool hasManualControl = false;

// void setup() {
//   Serial.begin(9600);
//   pinMode(LED_PIN, OUTPUT);
//   digitalWrite(LED_PIN, LOW);
//   Serial.println("✅ Arduino listo para control de LED via Serial con programación horaria");
// }

// void loop() {
//   handleSerial();
//   checkSchedule();
//   delay(1000);
// }

// void handleSerial() {
//   static String input = "";

//   while (Serial.available()) {
//     char c = Serial.read();

//     if (c == '\n') {
//       // 🔧 Workaround: limpiar cualquier basura antes del JSON
//       int jsonStart = input.indexOf('{');
//       if (jsonStart > 0) {
//         input = input.substring(jsonStart);  // Cortar desde '{'
//       }

//       Serial.print("📥 Recibido limpio: ");
//       Serial.println(input);

//       StaticJsonDocument<200> doc;
//       DeserializationError error = deserializeJson(doc, input);

//       if (!error) {
//         // Obtener hora actual
//         if (doc.containsKey("current_time")) {
//           String t = doc["current_time"]; // formato "HH:MM"
//           currentHour = t.substring(0, 2).toInt();
//           currentMinute = t.substring(3, 5).toInt();
//           Serial.print("⏰ Hora actual: ");
//           Serial.println(t);
//         }

//         // Control manual
//         if (doc.containsKey("led")) {
//           ledState = doc["led"];
//           hasManualControl = true;
//           digitalWrite(LED_PIN, ledState ? LOW : HIGH);
//           Serial.println(ledState ? "💡 LED APAGADO ❌ (manual)" : "💡 LED ENCENDIDO ✅ (manual)");
//         }

//         // Programación horaria
//         if (doc.containsKey("on_time") && doc.containsKey("off_time")) {
//           String on = doc["on_time"];
//           String off = doc["off_time"];
//           onHour = on.substring(0, 2).toInt();
//           onMinute = on.substring(3, 5).toInt();
//           offHour = off.substring(0, 2).toInt();
//           offMinute = off.substring(3, 5).toInt();
//           hasManualControl = false;

//           Serial.print("⏱️ Programado de ");
//           Serial.print(on);
//           Serial.print(" a ");
//           Serial.println(off);
//         }
//       } else {
//         Serial.print("❌ Error JSON: ");
//         Serial.println(error.c_str());
//       }

//       input = "";
//     } else {
//       input += c;
//     }
//   }
// }

// bool isBetween(int ch, int cm, int sh, int sm, int eh, int em) {
//   if (sh < eh || (sh == eh && sm < em)) {
//     return (ch > sh || (ch == sh && cm >= sm)) &&
//            (ch < eh || (ch == eh && cm < em));
//   } else {
//     return (ch > sh || (ch == sh && cm >= sm)) ||
//            (ch < eh || (ch == eh && cm < em));
//   }
// }

// void checkSchedule() {
//   if (hasManualControl || currentHour < 0 || onHour < 0) return;

//   bool shouldBeOn = isBetween(currentHour, currentMinute, onHour, onMinute, offHour, offMinute);
//   digitalWrite(LED_PIN, shouldBeOn ? LOW : HIGH);

//   Serial.print("⏰ Hora actual: ");
//   Serial.print(currentHour);
//   Serial.print(":");
//   Serial.println(currentMinute);
//   Serial.print("⏱️ Rango: ");
//   Serial.print(onHour);
//   Serial.print(":");
//   Serial.print(onMinute);
//   Serial.print(" -> ");
//   Serial.print(offHour);
//   Serial.print(":");
//   Serial.println(offMinute);
//   Serial.println(shouldBeOn ? "💡 Encendido automático ✅" : "💤 Apagado automático ❌");
// }
