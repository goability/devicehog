# ESP8266 Remote Control - AWS Lambda Backend

This Lambda function enables remote control of your ESP8266 from anywhere via a public webpage.

## Quick Start

### 1. Deploy to AWS

**Option A: AWS Console (No coding required)**

Follow the guide in `DEPLOY_INSTRUCTIONS.md`

**Option B: Serverless Framework**

```bash
# Install Serverless Framework
npm install -g serverless

# Deploy
cd lambda
serverless deploy
```

### 2. Configure ESP8266

In `nodemcu/nodemcu.ino`, update:

```cpp
const char* REMOTE_API_URL = "https://your-lambda-url.lambda-url.us-east-1.on.aws";
```

### 3. Use the Public Webpage

Open `public_control_example.html` and update the Lambda URL.

## How It Works

```
┌─────────────┐       ┌─────────────┐      ┌──────────────┐
│   Browser   │──────▶│  Lambda     │◀─────│   ESP8266    │
│  (Control)  │ POST  │  (API)      │ GET  │  (Device)     │
└─────────────┘       └─────────────┘      └──────────────┘
                              │
                              ▼
                       Command Queue
```

**Step-by-step:**
1. User clicks button on public webpage
2. Browser sends POST to Lambda with command
3. Lambda stores command in memory
4. ESP8266 polls Lambda every 5 seconds
5. Lambda returns command and clears it
6. ESP8266 executes command locally
7. ESP8266 reports success back to Lambda

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
Get the queued command for a device.

**Response:**
```json
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
  "status": "success",
  "timestamp": "123456"
}
```

## Available Commands

- `led1_on` - Turn LED1 on
- `led1_off` - Turn LED1 off
- `led2_on` - Turn LED2 on
- `led2_off` - Turn LED2 off
- `both_on` - Turn both LEDs on
- `both_off` - Turn both LEDs off

## Architecture

This Lambda uses **in-memory storage**, which means:
- ✅ Simple and fast
- ✅ No database setup needed
- ✅ Works immediately
- ⚠️ Commands cleared on cold start
- ⚠️ Not suitable for production at scale

### For Production

Replace in-memory storage with:
- **DynamoDB** - Recommended for persistence
- **ElastiCache (Redis)** - For speed + persistence  
- **S3** - Simple object storage

See `DEPLOY_INSTRUCTIONS.md` for production setup examples.

## Testing

### Test from command line:

```bash
# Send command
curl -X POST https://YOUR_LAMBDA_URL/device/command \
  -H "Content-Type: application/json" \
  -d '{"deviceId":"ESP8266_001","command":"led1_on"}'

# ESP8266 will receive it within 5 seconds
curl https://YOUR_LAMBDA_URL/device/commands?deviceId=ESP8266_001
```

### Test the webpage:

1. Open `public_control_example.html`
2. Update `API_URL` with your Lambda URL
3. Click buttons!

## Cost

**AWS Lambda Pricing:**
- **Free tier**: 1 million requests/month
- **After free tier**: $0.20 per million requests
- **24/7 ESP8266 polling** (every 5s): ~$1/month

**Function URL:**
- First 1 million requests/month: FREE
- Additional requests: Included with Lambda

## Security

Current setup:
- Function URL with Auth: NONE (public)
- CORS enabled for all origins
- In-memory storage (clears on cold start)

### Add Authentication

1. **Update Lambda** - Add API key check
2. **Set environment variable**: `API_KEY=your_secret`
3. **Update ESP8266** - Send API key in headers
4. **Update webpage** - Send API key in headers

See `DEPLOY_INSTRUCTIONS.md` for implementation.

## Files

- `esp8266-remote-control.js` - Lambda function code
- `package.json` - Node.js dependencies (none needed)
- `serverless.yml` - Serverless Framework config
- `DEPLOY_INSTRUCTIONS.md` - Step-by-step deployment guide

## Monitoring

View in AWS CloudWatch:
- Lambda metrics → Invocations, Duration, Errors
- Logs → All requests and responses

## Troubleshooting

**Commands not working?**
1. Check Function URL is correct in ESP8266
2. Check CloudWatch Logs for errors
3. Verify CORS is configured
4. Test endpoint manually with curl

**ESP8266 not connecting?**
1. Verify Lambda URL is reachable
2. Check WiFi connection
3. Monitor serial output for errors

**Authentication issues?**
1. Verify Function URL auth type: NONE
2. Check CORS configuration
3. Test with curl first

## Next Steps

1. ✅ Deploy Lambda function
2. ✅ Configure ESP8266 with Lambda URL
3. ✅ Test with command line (curl)
4. ✅ Test with public webpage
5. 🔒 Add authentication (optional)
6. 📊 Add monitoring/alerting
7. 🗄️ Migrate to DynamoDB (production)
