# # FROM nodered/node-red:latest

FROM nodered/node-red:3.1

# Cambiar a root para instalar
USER root

# Crear carpeta de usuario si no existe
RUN mkdir -p /data

# Copia config
COPY config.json /data/config.json

# Instalar nodos adicionales de Node-RED y dependencias JS necesarias
RUN npm install --unsafe-perm --no-update-notifier --no-fund \
    node-red-node-serialport \
    node-red-contrib-sails \
    # socket.io-client \
    # sails.io.js

# # Volver a usuario no-root
# USER node-red

# Imagen base oficial de Node-RED
# FROM nodered/node-red:3.1.0

# # Crea el directorio /data si no existe (para almacenamiento persistente de Node-RED)
# # RUN mkdir -p /data

# # Establece el directorio de trabajo
# WORKDIR /usr/src/node-red

# # Instala sails.io.js y socket.io-client@2.4.0 en el entorno de Node-RED
# # RUN npm install --prefix /usr/src/node-red sails.io.js socket.io-client@2.4.0
# RUN npm install --unsafe-perm --no-update-notifier --no-fund \
#     node-red-node-serialport \
#     node-red-contrib-sails \
#     # socket.io-client@2.0.4 \
#     # sails.io.js@1.2.1

# # Copiamos archivos a /usr/src/node-red (NO a /data)
# COPY config.json ./config.json
# # COPY settings.js ./settings.js

# # Define que usará el usuario por defecto de Node-RED
# # USER node-red
