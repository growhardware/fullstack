
.PHONY: up down logs restart rebuild emulator photo ai dev prod

# Levanta el stack con el entorno actual
up:
	docker compose --env-file .env up -d

down:
	docker compose down

logs:
	docker compose logs -f --tail=50

restart:
	docker compose down && docker compose --env-file .env up -d

rebuild:
	docker compose down && docker compose --env-file .env up --build -d

# Acceso al emulador desde navegador
emulator:
	open http://localhost:8082/capture || xdg-open http://localhost:8082/capture || echo "Visita http://localhost:8082/capture"

# Toma una foto del emulador y la guarda
photo:
	curl http://localhost:8082/capture --output ultima_foto.jpg && echo "📸 Foto guardada como ultima_foto.jpg"

# Prueba de endpoint AI-core
ai:
	curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" -d '{"data": [26.5, 87.0, 1]}'

# Entorno de desarrollo
dev:
	cp .env.development .env && docker compose --env-file .env up --build -d

# Entorno de producción (ESP32 real)
prod:
	cp .env.production .env && docker compose --env-file .env up --build -d
