# Placeholder: aquí irá el entrenamiento real cuando tengamos suficientes datos históricos.
import pandas as pd
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
import numpy as np
import os

# Cargar los datos simulados
df = pd.read_csv("data/optimize_inputs.csv")  # se espera este archivo

# X: temp, hum, especie_id, etapa_id
# Y: luz_hs, intervalo_riego_hs, duracion_riego_seg
X = df[['temp', 'hum', 'species_id', 'stage_id']].values
y = df[['light_hours', 'irrigation_interval', 'irrigation_duration']].values

model = Sequential([
    Dense(16, activation='relu', input_shape=(4,)),
    Dense(12, activation='relu'),
    Dense(3)  # salida: tres valores continuos
])

model.compile(optimizer='adam', loss='mse')
model.fit(X, y, epochs=100)

os.makedirs("model", exist_ok=True)
model.save("model/optimize_model.h5")
print("✅ Modelo optimizador guardado.")
