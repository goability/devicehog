# Remote Push Control for ESP8266

## 🚀 Quick Start with AWS Lambda (Recommended - 5 minutes)

**Easiest way to enable remote control!**

1. **Deploy Lambda:**
   ```bash
   cd lambda
   chmod +x deploy.sh
   ./deploy.sh
   ```
   Or follow `lambda/DEPLOY_INSTRUCTIONS.md` for manual setup.

2. **Update ESP8266** - Set `REMOTE_API_URL` to your Lambda Function URL

3. **Done!** - Your ESP8266 can be controlled remotely from anywhere

**Cost:** Free tier covers ~5 million requests/month

**Time:** 5 minutes to deploy

---

## How It Works

The ESP8266 uses a **polling pattern** to receive commands from outside your network:

1. **ESP8266 polls every 5 seconds** for new commands: `GET https://your-api.com/device/commands?deviceId=ESP8266_001`
2. **Public webpage** sends commands to your API: `POST https://your-api.com/device/command`
3. **API stores** the command temporarily
4. **ESP8266 retrieves** the command on next poll
5. **ESP8266 executes** the command and reports status back

## Setup Instructions

### 1. Configure the ESP8266

Edit line 52 in `nodemcu/nodemcu.ino`:

```cpp
const char* REMOTE_API_URL = "https://your-api-url.com/api";
```

### 2. Deploy an API Server

You need to create an API with these endpoints:

#### GET /device/commands?deviceId=ESP8266_001
Returns the current command for the device (or `null` if none).

**Example Response:**
```json
"led1_on"
```

#### POST /device/command
Accepts a new command to be sent to a device.

**Request Body:**
```json
{
  "deviceId": "ESP8266_001",
  "command": "led1_on"
}
```

#### POST /device/status
Receives execution status from the ESP8266.

**Request Body:**
```json
{
  "deviceId": "ESP8266_001",
  "command": "led1_on",
  "status": "success",
  "timestamp": "123456"
}
```

### 3. Example API Implementations

#### Option A: Node.js with Express

```javascript
const express = require('express');
const app = express();

let commands = {}; // Simple in-memory storage

app.use(express.json());

// Get command for a device
app.get('/device/commands', (req, res) => {
  const deviceId = req.query.deviceId;
  const command = commands[deviceId] || null;
  res.json(command);
  commands[deviceId] = null; // Clear after reading
});

// Store new command
app.post('/device/command', (req, res) => {
  const { deviceId, command } = req.body;
  commands[deviceId] = command;
  res.json({ success: true });
});

// Receive status updates
app.post('/device/status', (req, res) => {
  console.log('Device status:', req.body);
  res.json({ success: true });
});

app.listen(3000);
```

#### Option B: Firebase Realtime Database

```javascript
// JavaScript SDK
import { getDatabase, ref, get, set } from 'firebase/database';

// Get command
const command = await get(ref(db, `devices/${deviceId}/command`));
await set(ref(db, `devices/${deviceId}/command`), null); // Clear
return command;

// Store command
await set(ref(db, `devices/${deviceId}/command`), command);
```

#### Option C: Python Flask

```python
from flask import Flask, request, jsonify
app = Flask(__name__)

commands = {}

@app.route('/device/commands')
def get_command():
    device_id = request.args.get('deviceId')
    command = commands.get(device_id, None)
    commands[device_id] = None  # Clear
    return jsonify(command)

@app.route('/device/command', methods=['POST'])
def set_command():
    data = request.json
    commands[data['deviceId']] = data['command']
    return jsonify({'success': True})
```

### 4. Deploy Your API

- **Heroku**: Free tier available
- **Glitch**: Instant deployment
- **Vercel**: Serverless functions
- **Google Cloud Functions**: Free tier
- **Firebase**: Free tier
- **Your own server**: Any hosting

### 5. Use the Public Webpage

1. Open `public_control_example.html`
2. Update `API_URL` in the JavaScript section
3. Host it on GitHub Pages, Netlify, or your web server
4. Click buttons to control your ESP8266 remotely!

## Commands Available

- `led1_on` - Turn LED1 on
- `led1_off` - Turn LED1 off
- `led2_on` - Turn LED2 on
- `led2_off` - Turn LED2 off
- `both_on` - Turn both LEDs on
- `both_off` - Turn both LEDs off

## Architecture

```
Public Webpage (Internet)
    ↓
External API (Internet)
    ↑
ESP8266 (Your Network) - Polls every 5 seconds
```

This works because:
- ESP8266 makes **outbound** connections (no port forwarding needed)
- Works from behind NAT/firewalls
- Works on any WiFi network
- Secure (HTTPS)

## Troubleshooting

- **ESP8266 doesn't receive commands**: Check `REMOTE_API_URL` is set correctly
- **Commands not working**: Check your API is returning the correct format
- **Delayed response**: Commands are polled every 5 seconds (can be adjusted)
- **HTTPS errors**: Device uses `setInsecure()` for certificate validation (development only)

## Customizing

- Adjust polling interval: Change `REMOTE_POLL_INTERVAL` on line 50
- Add custom commands: Edit `checkRemoteCommands()` function
- Add authentication: Modify API to require API keys
