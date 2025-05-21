GrowHardware – Stack Dockerizado

1. Cloná tus proyectos reales en estas carpetas:
   - ./backend  (Sails.js)
   - ./io (frontend React + Tailwind)

2. Renombrá .env.sample a .env

3. Usá este comando para levantar todo:
   make up

Servicios:
- AI-core:                       http://localhost:8000
- Backend Sails:                 http://localhost:3000
- ESP32 Emulado:                 http://localhost:8082/capture
- MongoDB:                       http://localhost:27017
- Frontend React (.io):          http://localhost:8080
- Frontend Mkdocs (.org):        http://localhost:8001
- Frontend React (.foundation):  http://localhost:8002

Recordá configurar correctamente la conexión a Mongo desde el backend con:
  MONGO_URL=mongodb://mongodb:27017/growhardware

¡Buena demo!
