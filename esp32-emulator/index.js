const axios = require('axios');
const WebSocket = require('ws');
const express = require('express');
const fs = require('fs');

const API_URL = process.env.API_URL || 'http://localhost:3000';
const WS_URL = process.env.WS_URL || 'ws://localhost:3000/ws';
const EMAIL = process.env.EMAIL || 'hola@hola.io';
const PASSWORD = process.env.PASSWORD || 'hola';

let plan = {
  start_time: Math.floor(Date.now() / 1000),
  light_on_hours: 18,
  light_off_hours: 6,
  humidity_clone_min: 80,
  humidity_clone_max: 95,
  water: false
};

let lightOn = false;
let watered = false;

function isLightOn() {
  const now = Math.floor(Date.now() / 1000);
  const elapsed = now - plan.start_time;
  const cycle = plan.light_on_hours + plan.light_off_hours;
  return (elapsed % cycle) < plan.light_on_hours;
}

function simulateSensorData() {
  return {
    temp: (22 + Math.random() * 5).toFixed(1),
    hum: (75 + Math.random() * 10).toFixed(1),
    light: isLightOn()
  };
}

function connectWS(token) {
  const ws = new WebSocket(WS_URL + '?token=' + token);

  ws.on('open', () => {
    console.log('✅ WebSocket conectado');
  });

  ws.on('message', (msg) => {
    const data = JSON.parse(msg);
    if (data.type === 'plan:update') {
      plan = data.payload;
      watered = false;
      console.log('📥 Plan actualizado');
    }
  });

  setInterval(() => {
    const payload = simulateSensorData();
    ws.send(JSON.stringify({
      type: 'sensor_data',
      payload
    }));
    console.log('📤 Sensor data:', payload);
  }, 10000);
}

async function loginAndConnect() {
  try {
    const res = await axios.post(API_URL + '/api/v1/entrance/login', {
      emailAddress: EMAIL,
      password: PASSWORD
    });
    const token = res.data.token;
    connectWS(token);
  } catch (e) {
    console.error('❌ Error login:', e.message);
  }
}

// Mini servidor para simular /capture
const app = express();
app.get('/capture', (req, res) => {
  const img = fs.readFileSync('./camera.jpg');
  res.set('Content-Type', 'image/jpeg');
  res.send(img);
});
app.listen(8082, () => {
  console.log('📸 Simulación cámara en http://localhost:8082/capture');
});

loginAndConnect();