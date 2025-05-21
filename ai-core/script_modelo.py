import tensorflow as tf
import numpy as np
import os

# Datos ficticios para entrenamiento (temperatura, humedad, luz, aire, agua)
X_train = np.array([
    [24.0, 75, 1, 0, 0],
    [28.0, 65, 0, 1, 1],
    [22.0, 85, 1, 1, 0],
    [30.0, 60, 0, 0, 1],
])
y_train = np.array([1, 0, 1, 0])  # 1 = saludable, 0 = anomalía

model = tf.keras.models.Sequential([
    tf.keras.layers.Dense(10, activation='relu', input_shape=(5,)),
    tf.keras.layers.Dense(1, activation='sigmoid')
])

model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
model.fit(X_train, y_train, epochs=50, verbose=1)

# Guardar modelo
os.makedirs("model", exist_ok=True)
model.save("model/model.h5")
print("✅ Modelo entrenado y guardado en model/model.h5")
