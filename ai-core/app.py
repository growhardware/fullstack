from fastapi import FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/predict")
async def predict(request: Request):
    data = await request.json()
    # Simulación: si temp > 25 y hum > 85, recomienda bajar humedad
    temp = data['data'][0]
    hum = data['data'][1]
    light = data['data'][2]
    if temp > 25 and hum > 85:
        return {"action": "lower humidity"}
    elif temp < 20:
        return {"action": "increase temperature"}
    elif light == 0:
        return {"action": "turn on lights"}
    else:
        return {"action": "stable"}
        
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)