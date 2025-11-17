// AWS Lambda function for ESP8266 remote control
// Deploy this to Lambda with API Gateway or Function URL

// Simple in-memory storage
// In production, use DynamoDB, ElastiCache, or S3
const commands = {};

// HTML web interface
const htmlPage = `<!DOCTYPE html>
<html>
<head>
  <title>ESP8266 LED Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background: #f0f0f0;
    }
    .container {
      text-align: center;
      background: white;
      padding: 30px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 { color: #333; }
    .button {
      display: inline-block;
      padding: 15px 30px;
      margin: 10px;
      font-size: 18px;
      cursor: pointer;
      border: none;
      border-radius: 5px;
      text-decoration: none;
      color: white;
    }
    .btn-on { background: #4CAF50; }
    .btn-on:hover { background: #45a049; }
    .btn-off { background: #f44336; }
    .btn-off:hover { background: #da190b; }
    .btn-both { background: #2196F3; }
    .btn-both:hover { background: #0b7dda; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🌍 ESP8266 Remote Control</h1>
    <p>Click buttons to control your ESP8266 LEDs:</p>
    <div id="status"></div>
    <div style="margin-top: 20px;">
      <button class="button btn-on" onclick="sendCommand('led1_on')">LED1 ON</button>
      <button class="button btn-on" onclick="sendCommand('led2_on')">LED2 ON</button>
      <button class="button btn-both" onclick="sendCommand('both_on')">Both ON</button>
    </div>
    <div style="margin-top: 10px;">
      <button class="button btn-off" onclick="sendCommand('led1_off')">LED1 OFF</button>
      <button class="button btn-off" onclick="sendCommand('led2_off')">LED2 OFF</button>
      <button class="button btn-off" onclick="sendCommand('both_off')">Both OFF</button>
    </div>
  </div>
  
  <script>
    const API_URL = 'https://nnvz2ctqcvvpcmtevv6uqhllya0zktqc.lambda-url.us-east-1.on.aws';
    
    async function sendCommand(command) {
      const statusDiv = document.getElementById('status');
      
      try {
        statusDiv.innerHTML = '⏳ Sending command...';
        statusDiv.style.color = '#2196F3';
        
        const response = await fetch(API_URL + '/device/command', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({deviceId: 'ESP8266_001', command: command})
        });
        
        if (response.ok) {
          const data = await response.json();
          statusDiv.innerHTML = '✅ ' + data.message + ' - ESP8266 will receive within 5 seconds!';
          statusDiv.style.color = '#4CAF50';
        } else {
          throw new Error('Request failed');
        }
      } catch (error) {
        statusDiv.innerHTML = '❌ Error: Could not send command';
        statusDiv.style.color = '#f44336';
        console.error(error);
      }
    }
  </script>
</body>
</html>`;

exports.handler = async (event) => {
    // Handle both Function URL and API Gateway event structures
    let httpMethod, path, queryStringParameters, body;
    
    if (event.requestContext) {
        // Function URL event structure
        httpMethod = event.requestContext.http.method;
        path = event.rawPath || event.requestContext.http.path;
        queryStringParameters = event.queryStringParameters || {};
        body = event.body || '';
    } else {
        // API Gateway event structure
        httpMethod = event.httpMethod;
        path = event.path;
        queryStringParameters = event.queryStringParameters || {};
        body = event.body || '';
    }
    
    // Parse request
    const pathMatch = path.split('/');
    const endpoint = pathMatch[pathMatch.length - 1];
    const deviceId = queryStringParameters?.deviceId || (body ? JSON.parse(body).deviceId : null);
    
    try {
        // CORS headers for browser requests
        const headers = {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Headers': 'Content-Type',
            'Access-Control-Allow-Methods': 'GET,POST,OPTIONS',
            'Content-Type': 'application/json'
        };
        
        // Handle preflight OPTIONS request
        if (httpMethod === 'OPTIONS') {
            return {
                statusCode: 200,
                headers,
                body: ''
            };
        }
        
        // Route: GET / (root) - Serve HTML web interface
        if (httpMethod === 'GET' && (path === '/' || endpoint === '' || endpoint === 'device')) {
            return {
                statusCode: 200,
                headers: {
                    'Content-Type': 'text/html'
                },
                body: htmlPage
            };
        }
        
        // Route: GET /device/commands?deviceId=ESP8266_001
        if (httpMethod === 'GET' && endpoint === 'commands') {
            const command = commands[deviceId] || null;
            
            // Clear the command after reading (one-time use)
            if (commands[deviceId]) {
                delete commands[deviceId];
            }
            
            return {
                statusCode: 200,
                headers,
                body: command ? JSON.stringify(command) : JSON.stringify(null)
            };
        }
        
        // Route: POST /device/command
        if (httpMethod === 'POST' && endpoint === 'command') {
            const requestBody = JSON.parse(body || '{}');
            const { deviceId, command } = requestBody;
            
            if (!deviceId || !command) {
                return {
                    statusCode: 400,
                    headers,
                    body: JSON.stringify({ error: 'Missing deviceId or command' })
                };
            }
            
            // Store the command
            commands[deviceId] = command;
            
            console.log(`Command stored for ${deviceId}: ${command}`);
            
            return {
                statusCode: 200,
                headers,
                body: JSON.stringify({ 
                    success: true, 
                    message: 'Command queued',
                    command 
                })
            };
        }
        
        // Route: POST /device/status (receive status from ESP8266)
        if (httpMethod === 'POST' && endpoint === 'status') {
            const requestBody = JSON.parse(body || '{}');
            
            console.log('Device status received:', requestBody);
            
            // Log status for monitoring
            // In production, you might save to CloudWatch Logs or DynamoDB
            
            return {
                statusCode: 200,
                headers,
                body: JSON.stringify({ success: true, message: 'Status received' })
            };
        }
        
        // 404 - Unknown endpoint
        return {
            statusCode: 404,
            headers,
            body: JSON.stringify({ error: 'Not found' })
        };
        
    } catch (error) {
        console.error('Error:', error);
        
        return {
            statusCode: 500,
            headers: {
                'Access-Control-Allow-Origin': '*',
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ error: 'Internal server error' })
        };
    }
};
