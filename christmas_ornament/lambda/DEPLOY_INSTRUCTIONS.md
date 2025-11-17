# Deploy ESP8266 Remote Control to AWS Lambda

## Quick Deploy (5 minutes)

### Option 1: AWS Console (Easiest)

1. **Go to AWS Lambda Console**: https://console.aws.amazon.com/lambda/
2. **Click "Create function"**
3. **Choose "Author from scratch"**
4. **Function name**: `esp8266-remote-control`
5. **Runtime**: Node.js 18.x or 20.x
6. **Click "Create function"**

7. **Copy the code**: Open `esp8266-remote-control.js` and copy all content

8. **In Lambda console**, scroll down to "Code source" editor
9. **Replace** the template code with your copied code
10. **Click "Deploy"**

11. **Set up Function URL**:
    - Click "Configuration" tab → "Function URL"
    - Click "Create function URL"
    - Auth type: **NONE** (public access)
    - CORS: Enable
    - Click "Save"

12. **Copy the Function URL** (looks like: `https://xxxxx.lambda-url.us-east-1.on.aws/`)

### Option 2: AWS CLI

```bash
# Install AWS CLI if needed
# https://aws.amazon.com/cli/

# Configure AWS credentials
aws configure

# Create the Lambda function
aws lambda create-function \
  --function-name esp8266-remote-control \
  --runtime nodejs20.x \
  --role arn:aws:iam::YOUR_ACCOUNT:role/lambda-basic-execution \
  --handler esp8266-remote-control.handler \
  --zip-file fileb://lambda.zip \
  --region us-east-1

# Create Function URL
aws lambda create-function-url-config \
  --function-name esp8266-remote-control \
  --auth-type NONE \
  --cors '{"AllowOrigins":["*"],"AllowMethods":["GET","POST"],"AllowHeaders":["Content-Type"]}'
```

### Option 3: Serverless Framework

```bash
# Install serverless framework
npm install -g serverless

# Deploy (serverless.yml included)
cd lambda
serverless deploy
```

## Configure ESP8266

Update line 52 in `nodemcu/nodemcu.ino`:

```cpp
const char* REMOTE_API_URL = "https://YOUR_LAMBDA_URL.lambda-url.us-east-1.on.aws";
```

Replace `YOUR_LAMBDA_URL` with your actual Function URL.

## Test It

### Test from command line:

```bash
# Send a command
curl -X POST https://YOUR_LAMBDA_URL/device/command \
  -H "Content-Type: application/json" \
  -d '{"deviceId":"ESP8266_001","command":"led1_on"}'

# Check for commands (what ESP8266 polls)
curl https://YOUR_LAMBDA_URL/device/commands?deviceId=ESP8266_001
```

### Test the public webpage:

1. Open `public_control_example.html`
2. Update the JavaScript:
   ```javascript
   const API_URL = 'https://YOUR_LAMBDA_URL';
   ```
3. Open in browser and click buttons

## How It Works

```
Browser → Lambda Function URL → ESP8266 (polls every 5s)
```

**Flow:**
1. Public webpage sends command to Lambda
2. Lambda stores command in memory
3. ESP8266 polls Lambda every 5 seconds
4. Lambda returns command and clears it
5. ESP8266 executes command
6. ESP8266 reports status back to Lambda

## Production Improvements

For production use, replace in-memory storage with:

### Option 1: DynamoDB (Recommended)

```javascript
const AWS = require('aws-sdk');
const dynamodb = new AWS.DynamoDB.DocumentClient();

// Get command from DynamoDB
async function getCommand(deviceId) {
    const params = {
        TableName: 'ESP8266Commands',
        Key: { deviceId }
    };
    const result = await dynamodb.get(params).promise();
    return result.Item?.command || null;
}

// Store command in DynamoDB
async function storeCommand(deviceId, command) {
    const params = {
        TableName: 'ESP8266Commands',
        Item: { deviceId, command, timestamp: Date.now() }
    };
    await dynamodb.put(params).promise();
}
```

### Option 2: ElastiCache (Redis)

```javascript
const redis = require('redis');
const client = redis.createClient(process.env.REDIS_URL);

async function getCommand(deviceId) {
    const command = await client.get(deviceId);
    if (command) await client.del(deviceId);
    return command;
}

async function storeCommand(deviceId, command) {
    await client.set(deviceId, command, 'EX', 60); // 60 second TTL
}
```

### Option 3: S3

```javascript
const AWS = require('aws-sdk');
const s3 = new AWS.S3();

async function getCommand(deviceId) {
    try {
        const obj = await s3.getObject({
            Bucket: 'esp8266-commands',
            Key: deviceId
        }).promise();
        
        const command = obj.Body.toString();
        await s3.deleteObject({
            Bucket: 'esp8266-commands',
            Key: deviceId
        });
        
        return command;
    } catch (e) {
        return null;
    }
}
```

## Cost Estimate

**Free Tier (first 12 months):**
- 1 million requests/month FREE
- 400,000 GB-seconds compute time FREE

**After free tier:**
- $0.20 per million requests
- $0.0000166667 per GB-second

**Estimate for 24/7 ESP8266 polling:**
- ~5,184,000 requests/month (poll every 5s)
- **Cost: ~$1.04/month**

## Monitoring

View logs in CloudWatch:
- Go to Lambda function → Monitoring → View logs
- See all command executions and errors

## Troubleshooting

**Issue**: ESP8266 not receiving commands
- Check Function URL is correct in ESP8266 code
- Check Lambda logs for errors
- Verify CORS is enabled on Function URL

**Issue**: 403 Forbidden
- Check Function URL is configured with Auth type: NONE

**Issue**: Commands not clearing
- Lambda in-memory storage resets on cold starts
- Consider using DynamoDB for persistence

## Security (Optional)

Add API key authentication:

```javascript
const API_KEY = process.env.API_KEY;

if (event.headers['x-api-key'] !== API_KEY) {
    return {
        statusCode: 401,
        body: JSON.stringify({ error: 'Unauthorized' })
    };
}
```

Set environment variable in Lambda console: `API_KEY=your_secret_key`

Update ESP8266:
```cpp
http.addHeader("x-api-key", "your_secret_key");
```
