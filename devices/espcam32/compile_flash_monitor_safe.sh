#!/bin/bash

# ==============================================
# GrowHardware - Script avanzado ESP32-CAM
# Compila, flashea, desbloquea puerto y monitorea
# ==============================================

SKETCH_DIR="$HOME/dev/gh/GrowHardware_Full_Stack/devices/espcam32/propagation"
BIN_OUT="$SKETCH_DIR/build"
PORT="/dev/ttyUSB0"
BAUD="115200"
ARDUINO_CLI="$HOME/Arduino/bin/arduino-cli"

# 🔐 Verificar permisos sobre el puerto serial
if ! groups "$USER" | grep -q dialout; then
  echo "❌ Tu usuario '$USER' no pertenece al grupo 'dialout'."
  echo "   Ejecutá esto y luego cerrá sesión:"
  echo "   sudo usermod -a -G dialout $USER"
  exit 1
fi

# ⚙️ Verificar arduino-cli
if ! command -v "$ARDUINO_CLI" &> /dev/null; then
  echo "❌ arduino-cli no encontrado en $ARDUINO_CLI"
  exit 1
fi

# 🛑 Verificar si el puerto está siendo usado
PID=$(lsof -t "$PORT")
if [ -n "$PID" ]; then
  echo "⚠️ Puerto $PORT está siendo usado por proceso $PID. Cerrando..."
  kill -9 "$PID"
  sleep 1
fi

# 🧪 Compilar
echo "⚙️ Compilando $SKETCH_DIR ..."
"$ARDUINO_CLI" compile --fqbn esp32:esp32:esp32cam --output-dir "$BIN_OUT" "$SKETCH_DIR"
if [ $? -ne 0 ]; then
  echo "❌ Falló la compilación."
  exit 1
fi

BIN_FILE=$(find "$BIN_OUT" -name "*.bin" | head -n 1)
if [ ! -f "$BIN_FILE" ]; then
  echo "❌ Binario no encontrado tras compilar."
  exit 1
fi

# 🔥 Flashear
echo "🚀 Flasheando $BIN_FILE en $PORT a $BAUD..."
python3 -m esptool --chip esp32 --port "$PORT" --baud "$BAUD" write_flash -z 0x1000 "$BIN_FILE"
if [ $? -ne 0 ]; then
  echo "❌ Falló el flasheo. Verificá el cable USB o el botón BOOT."
  exit 1
fi

# 🔍 Monitor serial (preferencia: screen > tmux > miniterm)
echo "🔍 Intentando abrir monitor serial en $PORT a $BAUD..."

if command -v screen &> /dev/null; then
  echo "✅ Usando screen (Ctrl+A luego K para salir)"
  screen "$PORT" "$BAUD"
elif command -v tmux &> /dev/null; then
  echo "✅ Usando tmux"
  tmux new-session "cat $PORT"
elif python3 -c "import serial.tools.miniterm" &> /dev/null; then
  echo "✅ Usando Python miniterm"
  python3 -m serial.tools.miniterm "$PORT" "$BAUD"
else
  echo "⚠️ No se encontró screen, tmux ni miniterm. Instala uno para monitorear serial."
fi
