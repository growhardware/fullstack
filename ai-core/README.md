# GrowHardware AI-Core

Microservicio de inferencia con IA (TensorFlow) para la plataforma GrowHardware. Permite hacer predicciones sobre datos de sensores en tiempo real.

## 🧠 Entrenar el modelo (opcional)

Requiere Python 3.9+

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install tensorflow numpy
python script_modelo.py
```

Esto generará el modelo en `model/model.h5`.

## 🐳 Ejecutar con Docker

```bash
docker build -t growhardware/ai-core .
docker run -p 8000:8000 growhardware/ai-core
```

## 🧪 Test API

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"data": [25.0, 70, 1, 0, 0]}'
```


## ⚠️ Detección de Anomalías

```bash
python script_anomalias.py
```

Esto genera el modelo `autoencoder.h5` para el endpoint `/anomaly`.

### Test:
```bash
curl -X POST http://localhost:8000/anomaly \
  -H "Content-Type: application/json" \
  -d '{"data": [29.0, 40, 0, 1, 1]}'
```


## 🌿 Endpoint `/optimize`

Devuelve recomendaciones de horas de luz y riego a partir de los datos del entorno.

```bash
curl -X POST http://localhost:8000/optimize \
  -H "Content-Type: application/json" \
  -d '{"temp": 24, "hum": 78, "species_id": 0, "stage_id": 1}'
```

La primera versión usa reglas simples, pero va guardando los datos para autoentrenar un modelo ML con:

```bash
python script_optimize_train.py
```

Este modelo se usará automáticamente al estar disponible.
