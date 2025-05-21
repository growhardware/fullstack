#!/bin/bash

echo "🔍 Probing GrowHardware AI Core API..."

# Test de /
echo -n "GET / --> "
curl -s http://localhost:8000 | grep -q "GrowHardware AI-Core is running." && echo "✅ OK" || echo "❌ ERROR"

# Test /predict con datos válidos (5 features esperados por el modelo)
echo -n "POST /predict --> "
curl -s -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"data": [24.0, 65.0, 1, 1, 1]}' | grep -q "prediction" && echo "✅ OK" || echo "❌ ERROR"

# Test /anomaly con 5 features
echo -n "POST /anomaly --> "
curl -s -X POST http://localhost:8000/anomaly \
  -H "Content-Type: application/json" \
  -d '{"data": [25.0, 60.0, 1, 1, 1]}' | grep -q "anomaly" && echo "✅ OK" || echo "❌ ERROR"

# Test /optimize con datos mínimos
echo -n "POST /optimize --> "
curl -s -X POST http://localhost:8000/optimize \
  -H "Content-Type: application/json" \
  -d '{"temp": 24.0, "hum": 70, "species_id": 1, "stage_id": 2}' | grep -q "light_hours" && echo "✅ OK" || echo "❌ ERROR"
