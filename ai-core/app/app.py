from flask import Flask, request, jsonify
import tensorflow as tf
import numpy as np
import os
from pathlib import Path

app = Flask(__name__)

# Rutas de modelos
base_dir = Path(__file__).parent.parent
predict_model_path = base_dir / "model/model.h5"
auto_path = base_dir / "model/autoencoder.h5"
mean_path = base_dir / "model/anom_mean.npy"
std_path = base_dir / "model/anom_std.npy"
optimize_model_path = base_dir / "model/optimize_model.h5"

# Cargar modelo de predicción
predict_model = tf.keras.models.load_model(predict_model_path)

# Cargar autoencoder si existe
autoencoder = tf.keras.models.load_model(auto_path, compile=False) if auto_path.exists() else None
X_mean = np.load(mean_path) if mean_path.exists() else None
X_std = np.load(std_path) if std_path.exists() else None

@app.route('/')
def root():
    return "GrowHardware AI-Core is running."

@app.route('/predict', methods=['POST'])
def predict():
    try:
        data = request.json['data']
        prediction = predict_model.predict(np.array([data]))
        return jsonify({'prediction': float(prediction[0][0])})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/anomaly', methods=['POST'])
def anomaly():
    if autoencoder is None or X_mean is None or X_std is None:
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
    try:
        input_data = request.json
        temp = input_data.get('temp')
        hum = input_data.get('hum')
        species_id = input_data.get('species_id', 0)
        stage_id = input_data.get('stage_id', 0)

        os.makedirs("data", exist_ok=True)
        with open("data/optimize_inputs.csv", "a") as f:
            f.write(f"{temp},{hum},{species_id},{stage_id},NA,NA,NA\n")

        if optimize_model_path.exists():
            model = tf.keras.models.load_model(optimize_model_path)
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

    # Fallback
    light_hours = 18 if stage_id == 0 else 12
    irrigation_interval = 6 if hum < 70 else 12
    irrigation_duration = 30 if temp > 26 else 15

    return jsonify({
        'light_hours': light_hours,
        'irrigation_interval_hours': irrigation_interval,
        'irrigation_duration_sec': irrigation_duration,
        'from_model': False
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
