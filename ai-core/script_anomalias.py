import tensorflow as tf
import numpy as np
import os

# Datos de entrenamiento (solo ejemplos normales)
X_train = np.array([
    [24.0, 75, 1, 0, 0],
    [25.0, 70, 1, 0, 0],
    [23.5, 80, 1, 0, 0],
    [24.2, 72, 1, 0, 0],
])

# Normalización simple
X_mean = X_train.mean(axis=0)
X_std = X_train.std(axis=0)
X_norm = (X_train - X_mean) / X_std

# Guardar estadísticas
np.save("model/anom_mean.npy", X_mean)
np.save("model/anom_std.npy", X_std)

# Autoencoder
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(5,)),
    tf.keras.layers.Dense(4, activation='relu'),
    tf.keras.layers.Dense(3, activation='relu'),
    tf.keras.layers.Dense(4, activation='relu'),
    tf.keras.layers.Dense(5)
])

model.compile(optimizer='adam', loss='mse')
model.fit(X_norm, X_norm, epochs=100, verbose=1)

# Guardar modelo
model.save("model/autoencoder.h5")
print("✅ Autoencoder guardado en model/autoencoder.h5")
