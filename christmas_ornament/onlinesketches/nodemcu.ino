#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>  // WiFi configuration portal
#include <DNSServer.h>
#include <EEPROM.h>

/*
 * HOW TO CONNECT TO EXTERNAL IP ENDPOINTS:
 * 
 * 1. HTTP Request (GET):
 *    makeHTTPRequest("http://your-api.com/endpoint");
 * 
 * 2. POST with JSON:
 *    makePOSTRequest("http://your-api.com/data", "{\"key\":\"value\"}");
 * 
 * 3. Webhook (IFTTT, Zapier, etc.):
 *    callWebhook("https://maker.ifttt.com/trigger/event/json/with/key/YOUR_KEY", "data");
 * 
 * 4. HTTPS (Secure) - Uncomment lines in makeHTTPRequest():
 *    // Use WiFiClientSecure for HTTPS
 * 
 * Common Use Cases:
 * - Send data to cloud (Google Sheets, Firebase, etc.)
 * - Trigger IFTTT/Zapier automation
 * - Log to API
 * - Send emails via webhook
 * - Post to Slack/Discord
 * 
 * REMOTE PUSH CONTROL:
 * 
 * To enable remote control, the device polls an external service for commands:
 * 
 * 1. Device periodically checks: GET https://your-api.com/device/commands?deviceId=ESP8266
 * 2. API returns commands like: {"command":"led1_on"}
 * 3. Device executes command and reports status: POST to API
 * 4. Public webpage can send commands to the API
 * 5. Device will receive and execute them on next poll
 * 
 * Services that work:
 * - Firebase Realtime Database
 * - Google Cloud Functions
 * - Your own REST API
 * - IFTTT Applets
 * - Adafruit IO
 */

bool networkConnected = false;
IPAddress ipAddress;

// Remote control settings (polling pattern)
const unsigned long REMOTE_POLL_INTERVAL = 5000;  // Check for commands every 5 seconds
unsigned long lastRemotePoll = 0;

// AWS Lambda Function URL
const char* REMOTE_API_URL = "https://nnvz2ctqcvvpcmtevv6uqhllya0zktqc.lambda-url.us-east-1.on.aws";

const char* DEVICE_ID = "ESP8266_001";  // Your device ID

const int ledPin1 = D3; // Connect LED to D3
const int ledPin2 = D4; // Connect LED to D4

// WiFi Manager for configuration portal
WiFiManager wifiManager;

// Fallback credentials (optional - WiFiManager will handle most cases)
const char* fallback_ssid = "";
const char* fallback_password = "";

// Access Point name for configuration portal
const char* ap_name = "DeviceHog";

// Initialize the web server on port 8080
ESP8266WebServer server(8080);

// Authentication for public WiFi (set your own password)
const char* www_username = "admin";
const char* www_password = "password";  // CHANGE THIS!

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
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
    <h1>ESP8266 LED Control</h1>
    <p>Click buttons to control the LEDs:</p>
    <div>
      <a href="/led1/on" class="button btn-on">LED1 ON</a>
      <a href="/led2/on" class="button btn-on">LED2 ON</a>
      <a href="/both/on" class="button btn-both">Both ON</a>
    </div>
    <div style="margin-top: 10px;">
      <a href="/led1/off" class="button btn-off">LED1 OFF</a>
      <a href="/led2/off" class="button btn-off">LED2 OFF</a>
      <a href="/both/off" class="button btn-off">Both OFF</a>
    </div>
  </div>
</body>
</html>
)rawliteral";

// Check if client is authenticated
bool isAuthenticated() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

// Function to handle root (/) URL
void handleRoot() {
  if (!isAuthenticated()) return;
  server.send(200, "text/html", index_html);
}

// Function to handle LED1 ON
void handleLED1On() {
  if (!isAuthenticated()) return;
  digitalWrite(ledPin1, HIGH);  // HIGH = ON for active HIGH LEDs
  server.send(200, "text/html", index_html);
  Serial.println("LED1 turned ON");
}

// Function to handle LED1 OFF
void handleLED1Off() {
  if (!isAuthenticated()) return;
  digitalWrite(ledPin1, LOW);  // LOW = OFF for active HIGH LEDs
  server.send(200, "text/html", index_html);
  Serial.println("LED1 turned OFF");
}

// Function to handle LED2 ON
void handleLED2On() {
  if (!isAuthenticated()) return;
  digitalWrite(ledPin2, HIGH);  // HIGH = ON for active HIGH LEDs
  server.send(200, "text/html", index_html);
  Serial.println("LED2 turned ON");
}

// Function to handle LED2 OFF
void handleLED2Off() {
  if (!isAuthenticated()) return;
  digitalWrite(ledPin2, LOW);  // LOW = OFF for active HIGH LEDs
  server.send(200, "text/html", index_html);
  Serial.println("LED2 turned OFF");
}

// Function to handle Both ON
void handleBothOn() {
  if (!isAuthenticated()) return;
  digitalWrite(ledPin1, HIGH);  // HIGH = ON for active HIGH LEDs
  digitalWrite(ledPin2, HIGH);
  server.send(200, "text/html", index_html);
  Serial.println("Both LEDs turned ON");
}

// Function to handle Both OFF
void handleBothOff() {
  if (!isAuthenticated()) return;
  digitalWrite(ledPin1, LOW);  // LOW = OFF for active HIGH LEDs
  digitalWrite(ledPin2, LOW);
  server.send(200, "text/html", index_html);
  Serial.println("Both LEDs turned OFF");
}

// Function to reset WiFi credentials and restart config portal
void handleWiFiReset() {
  if (!isAuthenticated()) return;
  
  String html = "<html><body>";
  html += "<h1>WiFi Reset</h1>";
  html += "<p>WiFi credentials cleared. Restarting...</p>";
  html += "<p>The device will start in configuration mode.</p>";
  html += "<p>Connect to WiFi: <strong>" + String(ap_name) + "</strong></p>";
  html += "<p>Open browser to: <strong>http://192.168.4.1</strong></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  Serial.println("WiFi credentials reset - restarting...");
  
  delay(2000);
  wifiManager.resetSettings();  // Clear saved WiFi credentials
  ESP.restart();  // Restart to enter config mode
}

// Example: Make a GET request to an external API
// For HTTP:  makeHTTPRequest("http://your-api.com/endpoint");
// For HTTPS: makeHTTPSRequest("https://your-api.com/endpoint");
void makeHTTPRequest(const char* endpoint) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  WiFiClient client;

  Serial.print("Connecting to: ");
  Serial.println(endpoint);
  
  http.begin(client, endpoint);  // HTTP connection (port 80)
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

// Example: Make a GET request to an HTTPS endpoint (secure)
void makeHTTPSRequest(const char* endpoint) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  WiFiClientSecure client;
  
  // For HTTPS, skip certificate validation (not recommended for production)
  client.setInsecure();
  
  Serial.print("Connecting to: ");
  Serial.println(endpoint);
  
  if (http.begin(client, endpoint)) {
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("HTTPS Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + http.getString());
    } else {
      Serial.print("HTTPS Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("HTTPS connection failed!");
  }
}

// Example: Make a POST request with JSON data
// For HTTP: makePOSTRequest("http://your-api.com/data", jsonData);
// For HTTPS: use makePOSTSecureRequest() instead
void makePOSTRequest(const char* endpoint, const String& jsonData) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  WiFiClient client;

  Serial.print("POSTing to: ");
  Serial.println(endpoint);
  
  http.begin(client, endpoint);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonData);
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Response: " + http.getString());
  } else {
    Serial.print("Error: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

// Example: Make a secure POST request to HTTPS endpoint
void makePOSTSecureRequest(const char* endpoint, const String& jsonData) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // Skip certificate validation

  Serial.print("POSTing securely to: ");
  Serial.println(endpoint);
  
  if (http.begin(client, endpoint)) {
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      Serial.print("HTTPS Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + http.getString());
    } else {
      Serial.print("Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("HTTPS POST failed!");
  }
}

// Example: Call a webhook (like IFTTT, Zapier, etc.)
void callWebhook(const char* webhookURL, const String& eventName) {
  String url = String(webhookURL) + "?value1=" + eventName;
  makeHTTPRequest(url.c_str());
}

// Example usage - you can call this from your LED handlers
void notifyWebhook(String event) {
  // UNCOMMENT ONE OF THE EXAMPLES BELOW:
  
  // Example 1: IFTTT Webhook
  // callWebhook("https://maker.ifttt.com/trigger/led_on/json/with/key/YOUR_KEY", event);
  
  // Example 2: Zapier Webhook
  // makeHTTPRequest("https://hooks.zapier.com/hooks/catch/YOUR_WEBHOOK_ID/");
  
  // Example 3: Send to Google Sheets via Apps Script
  // makePOSTSecureRequest("https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec", 
  //   "{\"led\":\"" + event + "\",\"timestamp\":\"" + String(millis()) + "\"}");
  
  // Example 4: Send to a REST API
  // makePOSTSecureRequest("https://your-api.com/data", "{\"led\":\"" + event + "\"}");
  
  // Example 5: Simple HTTP GET request
  // makeHTTPRequest("http://your-server.com/log?event=" + event);
  
  Serial.println("Event: " + event);
}

// ===== REMOTE PUSH CONTROL FUNCTIONS =====

// Poll remote API for commands and execute them
void checkRemoteCommands() {
  if (strlen(REMOTE_API_URL) == 0) {
    return;  // Not configured
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // For HTTPS
  
  // Build URL with device ID
  String url = String(REMOTE_API_URL) + "/device/commands?deviceId=" + String(DEVICE_ID);
  
  Serial.print("Checking for remote commands: ");
  Serial.println(url);
  
  if (http.begin(client, url)) {
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String response = http.getString();
      Serial.print("Command WAS  received: ");
      Serial.println(response);
      
      // Parse and execute command
      // Note: LEDs are active HIGH (HIGH = ON, LOW = OFF)
      if (response.indexOf("led1_on") >= 0) {
        digitalWrite(ledPin1, HIGH);  // HIGH = ON for active HIGH LEDs
        Serial.println("Executed: LED1 ON");
        reportStatus("led1_on", "success");
      }
      else if (response.indexOf("led1_off") >= 0) {
        digitalWrite(ledPin1, LOW);  // LOW = OFF for active HIGH LEDs
        Serial.println("Executed: LED1 OFF");
        reportStatus("led1_off", "success");
      }
      else if (response.indexOf("led2_on") >= 0) {
        digitalWrite(ledPin2, HIGH);  // HIGH = ON for active HIGH LEDs
        Serial.println("Executed: LED2 ON");
        reportStatus("led2_on", "success");
      }
      else if (response.indexOf("led2_off") >= 0) {
        digitalWrite(ledPin2, LOW);  // LOW = OFF for active HIGH LEDs
        Serial.println("Executed: LED2 OFF");
        reportStatus("led2_off", "success");
      }
      else if (response.indexOf("both_on") >= 0) {
        digitalWrite(ledPin1, HIGH);  // HIGH = ON
        digitalWrite(ledPin2, HIGH);
        Serial.println("Executed: Both LEDs ON");
        reportStatus("both_on", "success");
      }
      else if (response.indexOf("both_off") >= 0) {
        digitalWrite(ledPin1, LOW);  // LOW = OFF
        digitalWrite(ledPin2, LOW);
        Serial.println("Executed: Both LEDs OFF");
        reportStatus("both_off", "success");
      }
      else if (response != "" && response != "null") {
        Serial.println("Unknown command: " + response);
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpCode);
    }
    
    http.end();
  }
}

// Report execution status back to the API
void reportStatus(String command, String status) {
  if (strlen(REMOTE_API_URL) == 0) {
    return;
  }
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  
  String url = String(REMOTE_API_URL) + "/device/status";
  String jsonData = "{\"deviceId\":\"" + String(DEVICE_ID) + "\",\"command\":\"" + command + "\",\"status\":\"" + status + "\",\"timestamp\":\"" + String(millis()) + "\"}";
  
  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonData);
    if (httpCode > 0) {
      Serial.println("Status reported: " + String(httpCode));
    }
    http.end();
  }
}

// Call this in your loop() function to enable remote control
void handleRemoteControl() {
  if (millis() - lastRemotePoll >= REMOTE_POLL_INTERVAL) {
    lastRemotePoll = millis();
    checkRemoteCommands();
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  
  // Initialize LEDs to OFF (active HIGH: HIGH = ON, LOW = OFF)
  digitalWrite(ledPin1, LOW);  // Start with LEDs OFF
  digitalWrite(ledPin2, LOW);

  Serial.println();
  Serial.println("=== ESP8266 WiFi Setup ===");
  
  // Initialize EEPROM for WiFiManager
  EEPROM.begin(512);
  
  // FORCE portal mode - clears saved credentials (remove this line after first setup)
  wifiManager.resetSettings();  // This will force the portal to appear
  
  // Configure WiFiManager
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  wifiManager.setConfigPortalTimeout(180);  // Portal timeout in seconds (3 minutes)
  
  // Optional: Set callback for when config portal starts
  wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.print("AP SSID: ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("\n📱 Connect your phone to WiFi:");
    Serial.println("   SSID: " + String(ap_name));
    Serial.println("   Open browser to: http://192.168.4.1");
    Serial.println("\n⏱️  You have 3 minutes to configure...");
  });
  
  // Try to connect with saved credentials, or start configuration portal
  Serial.println("Attempting to connect to WiFi...");
  if (!wifiManager.autoConnect(ap_name)) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    // Reset and try again
    ESP.restart();
  }
  
  Serial.println("");
  Serial.println("✅ WiFi connected!");

        ipAddress = WiFi.localIP();
  Serial.print("IP address: http://");
        Serial.println(ipAddress);
  Serial.println("Server started on port 8080");

  // Set up URL handlers
  server.on("/", handleRoot);
  server.on("/led1/on", handleLED1On);
  server.on("/led1/off", handleLED1Off);
  server.on("/led2/on", handleLED2On);
  server.on("/led2/off", handleLED2Off);
  server.on("/both/on", handleBothOn);
  server.on("/both/off", handleBothOff);
  server.on("/wifireset", handleWiFiReset);  // Reset WiFi credentials
  
  // Start server
  server.begin();
  }

void loop() {
  server.handleClient();  // Handle web server requests

  // Check for remote commands (polling pattern)
  handleRemoteControl();
}
