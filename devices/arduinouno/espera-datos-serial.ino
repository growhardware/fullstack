void setup() {
  Serial.begin(9600);
  Serial.println("🖥️ Esperando datos por Serial...");
}

void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      input.trim();  // Elimina \r y espacios
      Serial.print("📩 Mensaje recibido: ");
      Serial.println(input);
      input = "";  // Limpiar para el próximo
    } else {
      input += c;
    }
  }
}
