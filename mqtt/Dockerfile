# # Usar la imagen oficial de Mosquitto
# FROM eclipse-mosquitto:latest

# # Copiar el archivo de configuración personalizado si es necesario
# COPY mosquitto.conf /etc/mosquitto/mosquitto.conf

# # Exponer los puertos
# EXPOSE 1883 9001

# # Configurar el comando de inicio del contenedor
# CMD ["mosquitto", "-c", "/etc/mosquitto/mosquitto.conf"]


# FROM eclipse-mosquitto:latest

# Crear directorio de persistencia y asignar permisos
# RUN mkdir -p /var/lib/mosquitto/ && \
# chown -R mosquitto:mosquitto /var/lib/mosquitto/

# # Copiar configuración
# COPY mosquitto.conf /etc/mosquitto/mosquitto.conf

# # VOLUME /var/lib/mosquitto

# EXPOSE 1883 9001
# CMD ["mosquitto", "-c", "/etc/mosquitto/mosquitto.conf"]

# FROM eclipse-mosquitto:latest

# COPY mosquitto.conf etc/mosquitto/mosquitto.conf

# # ARG USERNAME
# # ARG PASSWORD

# # RUN mosquitto_passwd -c -b passwd $USERNAME $PASSWORD

# EXPOSE 1883

FROM eclipse-mosquitto:latest
COPY mosquitto.conf /etc/mosquitto/mosquitto.conf
EXPOSE 1883
# 9001
CMD ["mosquitto", "-c", "/etc/mosquitto/mosquitto.conf"]