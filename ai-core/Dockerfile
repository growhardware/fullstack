FROM python:3.9-slim

WORKDIR /app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY app ./app
COPY data ./data
COPY model ./model
# COPY model/anom_mean.npy model/anom_mean.npy

COPY script_anomalias.py /script_anomalias.py
RUN python3 /script_anomalias.py
COPY script_modelo.py /script_modelo.py
RUN python3 /script_modelo.py
COPY script_optimize_train.py /script_optimize_train.py
RUN python3 /script_optimize_train.py

ENV PYTHONUNBUFFERED=1
CMD ["python", "app/app.py"]
