from flask import Flask, request, jsonify
import tensorflow as tf
import numpy as np
import os

app = Flask(__name__)
model_path = os.path.join(os.path.dirname(__file__), '../model/model.h5')
model = tf.keras.models.load_model(model_path)

@app.route('/predict', methods=['POST'])
def predict():
    try:
        data = request.json['data']
        prediction = model.predict(np.array([data]))
        return jsonify({'prediction': float(prediction[0][0])})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/', methods=['GET'])
def root():
    return "GrowHardware AI-Core is running."

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
import numpy as np

# Cargar autoencoder y stats si existen
from pathlib import Path
auto_path = Path(__file__).parent.parent / "model/autoencoder.h5"
mean_path = Path(__file__).parent.parent / "model/anom_mean.npy"
std_path = Path(__file__).parent.parent / "model/anom_std.npy"

if auto_path.exists():
    autoencoder = tf.keras.models.load_model(auto_path)
    X_mean = np.load(mean_path)
    X_std = np.load(std_path)
else:
    autoencoder = None

@app.route('/anomaly', methods=['POST'])
def anomaly():
    if autoencoder is None:
        return jsonify({'error': 'Modelo no entrenado'}), 500

    try:
        data = np.array(request.json['data'])
        norm = (data - X_mean) / X_std
        recon = autoencoder.predict(np.array([norm]))
        loss = tf.keras.losses.mse(norm, recon[0]).numpy().mean()
        is_anomaly = loss > 0.5
        return jsonify({'anomaly': bool(is_anomaly), 'score': float(loss)})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/optimize', methods=['POST'])
def optimize():
    input_data = request.json
    temp = input_data.get('temp')
    hum = input_data.get('hum')
    species_id = input_data.get('species_id', 0)
    stage_id = input_data.get('stage_id', 0)

    # Guardar input para futura mejora
    os.makedirs("data", exist_ok=True)
    with open("data/optimize_inputs.csv", "a") as f:
        f.write(f"{temp},{hum},{species_id},{stage_id},NA,NA,NA\n")

    # Cargar modelo si existe
    try:
        model_path = Path(__file__).parent.parent / "model/optimize_model.h5"
        if model_path.exists():
            model = tf.keras.models.load_model(model_path)
            input_arr = np.array([[temp, hum, species_id, stage_id]])
            pred = model.predict(input_arr)[0]
            return jsonify({
                'light_hours': float(pred[0]),
                'irrigation_interval_hours': float(pred[1]),
                'irrigation_duration_sec': float(pred[2]),
                'from_model': True
            })
    except Exception as e:
        print("❌ Error usando modelo optimizador:", e)

    # Fallback: lógica por especie y etapa
    light_hours = 18 if stage_id == 0 else 12
    irrigation_interval = 6 if hum < 70 else 12
    irrigation_duration = 30 if temp > 26 else 15

    return jsonify({
        'light_hours': light_hours,
        'irrigation_interval_hours': irrigation_interval,
        'irrigation_duration_sec': irrigation_duration,
        'from_model': False
    })
