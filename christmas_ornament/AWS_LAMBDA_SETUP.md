# ESP8266 Remote Control with AWS Lambda

## Overview

This setup enables remote control of your ESP8266 from anywhere via a public webpage, using AWS Lambda as the backend API.

**Key Benefits:**
- ✅ Serverless (no infrastructure to manage)
- ✅ Free tier: 1 million requests/month
- ✅ Works from behind NAT/firewalls
- ✅ HTTPS by default
- ✅ Deploy in 5 minutes

## Architecture

```
┌─────────────────┐
│  Public Webpage │
│  (Any browser)  │
└────────┬────────┘
         │ POST command
         ▼
┌─────────────────┐
│  AWS Lambda     │
│  (Function URL) │
└────────┬────────┘
         │ GET command (poll)
         │
         ▼
┌─────────────────┐
│    ESP8266      │
│  (Your Network) │
└─────────────────┘
```

## Quick Start

### Step 1: Deploy Lambda (5 minutes)

**Option A: AWS Console (No CLI needed)**

1. Go to https://console.aws.amazon.com/lambda/
2. Click "Create function"
3. Choose "Author from scratch"
4. Name: `esp8266-remote-control`
5. Runtime: Node.js 20.x
6. Click "Create function"

7. **Copy code**: Open `lambda/esp8266-remote-control.js` and copy all content

8. **Paste** into Lambda code editor

9. **Click "Deploy"**

10. **Create Function URL**:
    - Configuration tab → Function URL
    - Create function URL
    - Auth type: **NONE**
    - CORS: Enable
    - Save

11. **Copy the Function URL**

**Option B: Use deploy script**

```bash
cd lambda
chmod +x deploy.sh
./deploy.sh
```

The script will output your Function URL.

### Step 2: Configure ESP8266

Edit `nodemcu/nodemcu.ino` line 55:

```cpp
const char* REMOTE_API_URL = "https://YOUR_LAMBDA_URL.lambda-url.us-east-1.on.aws";
```

Replace `YOUR_LAMBDA_URL` with your actual Function URL.

### Step 3: Upload ESP8266 Sketch

```bash
cd /Users/matt/workspace/dev/devicehog/christmas_ornament
./bin/arduino-cli upload -p /dev/cu.SLAB_USBtoUART --fqbn esp8266:esp8266:nodemcuv2 nodemcu/nodemcu.ino
```

### Step 4: Test

**Test from command line:**

```bash
# Send a command
curl -X POST https://YOUR_LAMBDA_URL/device/command \
  -H "Content-Type: application/json" \
  -d '{"deviceId":"ESP8266_001","command":"led1_on"}'

# ESP8266 will receive it within 5 seconds!
```

**Test with public webpage:**

1. Open `public_control_example.html`
2. Update `API_URL` with your Lambda URL
3. Click buttons
4. Watch ESP8266 respond!

## How It Works

### Flow

1. **User clicks button** on public webpage
2. **Browser sends POST** to Lambda with command
3. **Lambda stores** command in memory
4. **ESP8266 polls** Lambda every 5 seconds
5. **Lambda returns** command and clears it
6. **ESP8266 executes** command locally
7. **ESP8266 reports** status back to Lambda

### Polling Pattern

The ESP8266 uses a **polling pattern** because:
- ✅ Works from behind NAT (no port forwarding)
- ✅ Works on any network (home, public WiFi, etc.)
- ✅ No incoming connections needed
- ✅ Simple and reliable

The device checks for commands every 5 seconds.

## Commands

Available commands:
- `led1_on` / `led1_off`
- `led2_on` / `led2_off`
- `both_on` / `both_off`

Add custom commands by editing `checkRemoteCommands()` in the ESP8266 code.

## API Endpoints

### POST /device/command

Store a command for a device.

**Request:**
```json
{
  "deviceId": "ESP8266_001",
  "command": "led1_on"
}
```

**Response:**
```json
{
  "success": true,
  "command": "led1_on"
}
```

### GET /device/commands?deviceId=ESP8266_001

Get the queued command.

**Response:**
```
"led1_on"
```

Or `null` if no command.

### POST /device/status

Device reports execution status.

**Request:**
```json
{
  "deviceId": "ESP8266_001",
  "command": "led1_on",
  "status": "success"
}
```

## Files Created

```
christmas_ornament/
├── lambda/
│   ├── esp8266-remote-control.js  # Lambda function
│   ├── package.json
│   ├── serverless.yml
│   ├── deploy.sh                  # Quick deploy script
│   ├── README.md                  # Lambda docs
│   └── DEPLOY_INSTRUCTIONS.md     # Step-by-step guide
├── nodemcu/
│   └── nodemcu.ino                # ESP8266 code (updated)
├── public_control_example.html    # Public webpage
└── README_REMOTE_CONTROL.md      # General docs
```

## Cost

**Free Tier (first 12 months):**
- 1,000,000 requests/month: FREE
- 400,000 GB-seconds: FREE

**After free tier:**
- Requests: $0.20 per million
- Compute: $0.0000166667 per GB-second

**24/7 ESP8266 polling:**
- ~5,184,000 requests/month
- **Cost: ~$1.04/month**

## Production Improvements

The current Lambda uses **in-memory storage**. For production:

### Add DynamoDB (Recommended)

1. Create DynamoDB table: `esp8266-commands`
2. Update Lambda code to use DynamoDB
3. Add IAM permissions

See `lambda/DEPLOY_INSTRUCTIONS.md` for examples.

### Add Authentication

1. Set Lambda environment variable: `API_KEY`
2. Update Lambda to check headers
3. Update ESP8266 to send API key
4. Update webpage to send API key

## Monitoring

**View logs:**
- AWS Lambda Console → Your function → Monitoring → View logs

**Set up alerts:**
- CloudWatch Alarms for errors
- SNS notifications

## Troubleshooting

**ESP8266 not receiving commands:**
1. Check Lambda URL in ESP8266 code
2. Check CloudWatch Logs for errors
3. Test endpoint with curl
4. Monitor serial output

**CORS errors:**
1. Enable CORS in Function URL config
2. Add wildcard origin

**Lambda cold starts:**
- In-memory storage resets on cold start
- Use DynamoDB for persistence

## Security

**Current setup:** Public access (anyone can control)

**For production:**
1. Add API key authentication
2. Enable AWS WAF
3. Use AWS API Gateway with API keys
4. Add rate limiting

## Next Steps

1. ✅ Deploy Lambda
2. ✅ Configure ESP8266
3. ✅ Test locally
4. 🔒 Add authentication
5. 📊 Add monitoring
6. 🗄️ Migrate to DynamoDB
7. 🚀 Deploy public webpage
8. 📱 Create mobile app

## Support

- Lambda logs: CloudWatch Logs
- ESP8266 logs: Serial monitor (115200 baud)
- See `lambda/DEPLOY_INSTRUCTIONS.md` for detailed setup
- See `lambda/README.md` for Lambda-specific docs
