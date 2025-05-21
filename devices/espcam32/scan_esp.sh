#!/bin/bash

# --------------------------------------------
# scan_esp.sh - Detecta ESP32-CAM en la red
# --------------------------------------------

echo "🔍 Detectando subred local..."
IP=$(ip a | grep 'inet ' | grep -v '127.0.0.1' | grep -v 'docker' | grep -v '172.' | awk '{print $2}' | head -n 1)
SUBNET=$(echo $IP | cut -d'/' -f1 | cut -d'.' -f1-3).0/24

echo "📡 Escaneando red: $SUBNET ..."
if ! command -v nmap &> /dev/null; then
  echo "❌ nmap no está instalado. Ejecutá: sudo apt install nmap"
  exit 1
fi

echo
nmap -sn $SUBNET | tee /tmp/nmap_esp_scan.txt | grep -B 2 -i "Espressif" || echo "❌ No se detectó ningún dispositivo ESP32 (Espressif)."

echo
echo "🧪 ¿Querés probar acceder al /log de los dispositivos encontrados?"
IPS=$(grep -B 2 -i "Espressif" /tmp/nmap_esp_scan.txt | grep 'Nmap scan report' | awk '{print $5}')
for ip in $IPS; do
  echo "🌐 Probing http://$ip/log ..."
  curl -m 2 http://$ip/log || echo "❌ Sin respuesta de $ip"
  echo
done
