#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  // WiFi configuration portal
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

/*
 * LOCAL ESP8266 LED CONTROL
 * 
 * This device runs ONLY locally on your network.
 * It connects to WiFi and serves a local web page on port 8080
 * to control two LEDs directly from any device on your network.
 * 
 * No external API connections or remote control.
 */

 // Access Point name for configuration portal
 // ** THIS MUST BE UNIQUE to allow multiple ornaments to be controlled **
const char* ap_name = "Ornament";



bool networkConnected = false;
bool apMode = true;  // Start in AP mode by default
IPAddress ipAddress;
String currentSSID = "";

// Current RGB LED values (0-255)
int currentR = 0;
int currentG = 0;
int currentB = 0;

// LED1 fade control variables
bool led1FadeEnabled = false;
int led1FadeSpeed = 10;  // Delay in milliseconds between fade steps (1-100)
int led1Brightness = 0;  // Current brightness (0-255)
int led1FadeDirection = 1;  // 1 = fading up, -1 = fading down
unsigned long lastFadeUpdate = 0;

const int ledPin1 = D1; // RED LED
const int ledPin2 = D2; // GREEN LED

// RGB LED pins
const int rgbRedPin = D8;   // Red channel
const int rgbGreenPin = D7; // Green channel
const int rgbBluePin = D6;  // Blue channel

// WiFi Manager for configuration portal
WiFiManager wifiManager;

// Fallback credentials (optional - WiFiManager will handle most cases)
const char* fallback_ssid = "";
const char* fallback_password = "";



// Initialize the web server on port 8080
ESP8266WebServer server(8080);

// Authentication for public WiFi (set your own password)
const char* www_username = "admin";
const char* www_password = "password";  // CHANGE THIS!

// HTML web page
// AUTO-GENERATED WEB CONTENT START - DO NOT EDIT MANUALLY
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Ornament Control Panel</title>
  <link rel="stylesheet" href="style.css">
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
.button-small {
  display: inline-block;
  text-decoration: none;
  border-radius: 2px;
  cursor:pointer;
  padding: 2px 2px;
  font-size: 8px;
}
.btn-on { background: #4CAF50; }
.btn-on:hover { background: #45a049; }
.btn-off { background: #f44336; }
.btn-off:hover { background: #da190b; }
.btn-both { background: #2196F3; }
.btn-both:hover { background: #0b7dda; }
.rgb-section {
  margin-top: 30px;
  padding-top: 20px;
  border-top: 2px solid #ddd;
}
.rgb-label {
  font-size: 16px;
  font-weight: bold;
  margin-bottom: 10px;
  color: #333;
}
#colorPicker {
  width: 100%;
  height: 50px;
  border: 2px solid #ddd;
  border-radius: 5px;
  cursor: pointer;
}
.btn-rgb {
  background: #9C27B0;
  margin-top: 10px;
}
.btn-rgb:hover { background: #7B1FA2; }


  </style>
</head>
<body>
  <div class="container">
    <h1>Ornament Control Panel</h1>
  <div class="rgb-section">
    <div class="rgb-label">RGB LED Control</div>
    <div style="margin-top: 10px; display: flex; gap: 10px; justify-content: center; flex-wrap: wrap;">
      <a href="/rgb?r=255&g=0&b=0" class="button" style="background: #ff0000; min-width: 100px;">RED</a>
      <a href="/rgb?r=0&g=255&b=0" class="button" style="background: #00ff00; min-width: 100px;">GREEN</a>
      <a href="/rgb?r=0&g=0&b=255" class="button" style="background: #0000ff; min-width: 100px;">BLUE</a>
    </div>
    <div style="margin-top: 10px;">
      <a href="/rgb/off" class="button btn-off">Lights OFF</a>
    </div>
  </div>
  <div style="margin-top: 20px; padding-top: 20px; border-top: 1px solid #ddd;">
    <div class="rgb-label">LED 1 Control</div>
    <a href="/led1/on" class="button btn-on">LED 1 ON</a>
    <a href="/led1/off" class="button btn-off">LED 1 OFF</a>
    <div style="margin-top: 15px;">
      <div style="margin-bottom: 10px;">
        <label for="fadeSpeed" style="display: block; margin-bottom: 5px; font-weight: bold;">Fade Speed: <span id="speedValue">10</span> ms</label>
        <input type="range" id="fadeSpeed" min="1" max="100" value="10" style="width: 100%;" oninput="updateFadeSpeed(this.value)">
      </div>
      <a href="#" onclick="startFade(); return false;" class="button btn-on" style="margin-right: 10px;">Start Fade</a>
      <a href="#" onclick="stopFade(); return false;" class="button btn-off">Stop Fade</a>
    </div>
  </div>
  <div style="margin-top: 20px; padding-top: 20px; border-top: 1px solid #ddd;">
    <a href="/info" class="button-small" style="background: #2196F3;">Device Info</a>
    <a href="/wifistatus" class="button-small" style="background: #9C27B0;">WiFi Settings</a>
    <a href="/wifireset" class="button-small" style="background: #FF9800;">Reset WiFi</a>
    <a href="/rgbtest" class="button-small" style="background: #9C27B0;">RGB Test</a>
  </div>
  </div>
  
  <script>
    // RGB color button handlers - buttons use direct links, no JS needed
    
    // LED1 fade control functions
    function updateFadeSpeed(speed) {
      document.getElementById('speedValue').textContent = speed;
      // Send speed to server
      fetch('/led1/fadespeed?speed=' + speed)
        .then(response => response.text())
        .then(data => console.log('Fade speed updated:', data))
        .catch(error => console.error('Error updating fade speed:', error));
    }
    
    function startFade() {
      fetch('/led1/fade?action=start')
        .then(response => response.text())
        .then(data => {
          console.log('Fade started:', data);
          alert('Fade started!');
        })
        .catch(error => {
          console.error('Error starting fade:', error);
          alert('Error starting fade');
        });
    }
    
    function stopFade() {
      fetch('/led1/fade?action=stop')
        .then(response => response.text())
        .then(data => {
          console.log('Fade stopped:', data);
          alert('Fade stopped!');
        })
        .catch(error => {
          console.error('Error stopping fade:', error);
          alert('Error stopping fade');
        });
    }
    
    // Page loaded
    window.onload = function() {
      // Any initialization code can go here
    };
    
    
  </script>
</body>
</html>

)rawliteral";
// AUTO-GENERATED WEB CONTENT END

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
  led1FadeEnabled = false;  // Stop fading
  digitalWrite(ledPin1, HIGH);  // HIGH = ON for active HIGH LEDs
  led1Brightness = 255;
  server.send(200, "text/html", index_html);
  Serial.println("LED1 turned ON");
}

// Function to handle LED1 OFF
void handleLED1Off() {
  if (!isAuthenticated()) return;
  led1FadeEnabled = false;  // Stop fading
  digitalWrite(ledPin1, LOW);  // LOW = OFF for active HIGH LEDs
  led1Brightness = 0;
  server.send(200, "text/html", index_html);
  Serial.println("LED1 turned OFF");
}

// Function to handle LED1 fade speed setting
void handleLED1FadeSpeed() {
  if (!isAuthenticated()) return;
  String speed_str = server.arg("speed");
  int speed = speed_str.toInt();
  // Clamp speed to 1-100 milliseconds
  led1FadeSpeed = constrain(speed, 1, 100);
  server.send(200, "text/plain", "OK");
  Serial.printf("LED1 fade speed set to %d ms\n", led1FadeSpeed);
}

// Function to handle LED1 fade enable/disable
void handleLED1Fade() {
  if (!isAuthenticated()) return;
  String action = server.arg("action");
  if (action == "start") {
    led1FadeEnabled = true;
    led1Brightness = 0;
    led1FadeDirection = 1;
    server.send(200, "text/plain", "Fade started");
    Serial.println("LED1 fade started");
  } else if (action == "stop") {
    led1FadeEnabled = false;
    analogWrite(ledPin1, led1Brightness);
    server.send(200, "text/plain", "Fade stopped");
    Serial.println("LED1 fade stopped");
  } else {
    server.send(400, "text/plain", "Invalid action");
  }
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

// Function to show device info (IP address)
void handleInfo() {
  if (!isAuthenticated()) return;
  
  String html = "<!DOCTYPE html><html><head><title>Device Info</title>";
  html += "<style>body{font-family:Arial,sans-serif;padding:30px;background:#f0f0f0;}";
  html += ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:10px;}";
  html += ".info{background:#e8f5e9;padding:15px;border-radius:5px;margin:10px 0;border-left:4px solid #4CAF50;}";
  html += ".url{background:#fff;padding:10px;font-family:monospace;word-break:break-all;border:1px solid #ddd;border-radius:3px;}";
  html += "a{color:#4CAF50;text-decoration:none;font-weight:bold;}</style></head><body>";
  html += "<div class='container'><h1>📡 Device Information</h1>";
  html += "<div class='info'><strong>Device Name:</strong><br>Ornament</div>";
  html += "<div class='info'><strong>WiFi Network:</strong><br>" + String(WiFi.SSID()) + "</div>";
  html += "<div class='info'><strong>IP Address:</strong><br><div class='url'>" + ipAddress.toString() + "</div></div>";
  html += "<div class='info'><strong>Access URL:</strong><br><div class='url'>http://" + ipAddress.toString() + ":8080</div></div>";
  html += "<div class='info'><strong>mDNS URL (alternative):</strong><br><div class='url'>http://ornament.local:8080</div></div>";
  html += "<div class='info'><strong>Username:</strong> admin<br><strong>Password:</strong> " + String(www_password) + "</div>";
  html += "<p style='text-align:center;margin-top:20px;'><a href='/'>← Back to Control Panel</a></p>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Function to handle RGB LED color setting
void handleRGBColor() {
  if (!isAuthenticated()) return;
  
  String r_str = server.arg("r");
  String g_str = server.arg("g");
  String b_str = server.arg("b");
  
  int r = r_str.toInt();
  int g = g_str.toInt();
  int b = b_str.toInt();
  
  // Clamp values to 0-255 range
  r = constrain(r, 0, 255);
  g = constrain(g, 0, 255);
  b = constrain(b, 0, 255);
  
  // Store current values
  currentR = r;
  currentG = g;
  currentB = b;
  
  // Write PWM values to RGB LED pins (common cathode - direct PWM)
  analogWrite(rgbRedPin, r);
  analogWrite(rgbGreenPin, g);
  analogWrite(rgbBluePin, b);
  
  // Redirect back to home with current RGB values to preserve color picker state
  server.sendHeader("Location", "/?r=" + String(r) + "&g=" + String(g) + "&b=" + String(b));
  server.send(302, "text/plain", "");
  Serial.printf("RGB LED set to (R:%d G:%d B:%d)\n", r, g, b);
}

// Function to handle RGB LED OFF
void handleRGBOff() {
  if (!isAuthenticated()) return;
  
  // Store current values
  currentR = 0;
  currentG = 0;
  currentB = 0;
  
  // For common cathode LEDs, write 0 to turn OFF
  analogWrite(rgbRedPin, 0);
  analogWrite(rgbGreenPin, 0);
  analogWrite(rgbBluePin, 0);
  
  // Redirect back to home with RGB OFF values
  server.sendHeader("Location", "/?r=0&g=0&b=0");
  server.send(302, "text/plain", "");
  Serial.println("RGB LED turned OFF");
}

// Function to test RGB LED type (common cathode vs common anode)
void handleRGBTest() {
  if (!isAuthenticated()) return;
  
  String html = "<!DOCTYPE html><html><head><title>RGB Test</title>";
  html += "<style>body{font-family:Arial,sans-serif;padding:30px;background:#f0f0f0;}";
  html += ".container{max-width:600px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:10px;}";
  html += ".button{padding:15px 30px;margin:10px;font-size:18px;cursor:pointer;border:none;border-radius:5px;color:white;text-decoration:none;display:inline-block;}";
  html += ".btn-test{background:#2196F3;} .btn-test:hover{background:#0b7dda;}";
  html += ".info{background:#e8f5e9;padding:15px;border-radius:5px;margin:10px 0;border-left:4px solid #4CAF50;}";
  html += "</style></head><body><div class='container'>";
  html += "<h1>RGB LED Type Test</h1>";
  html += "<div class='info'><strong>Instructions:</strong><br>";
  html += "Click each button and observe the LED behavior to determine the LED type.</div>";
  html += "<p><a href='/rgbtest?mode=red' class='button btn-test'>Test RED (255,0,0)</a></p>";
  html += "<p><a href='/rgbtest?mode=green' class='button btn-test'>Test GREEN (0,255,0)</a></p>";
  html += "<p><a href='/rgbtest?mode=blue' class='button btn-test'>Test BLUE (0,0,255)</a></p>";
  html += "<p><a href='/rgbtest?mode=alloff' class='button btn-test'>Turn ALL OFF</a></p>";
  html += "<div class='info'><strong>Current Status:</strong><br>";
  
  // Get test mode from URL parameter
  String mode = server.arg("mode");
  
  if (mode == "red") {
    // Test: Write HIGH (255) to red pin for common cathode
    analogWrite(rgbRedPin, 255);
    analogWrite(rgbGreenPin, 0);
    analogWrite(rgbBluePin, 0);
    html += "Testing with RED pin at PWM=255. LED should light red (COMMON CATHODE).";
    Serial.println("RGB TEST: Red test - writing PWM=255 to red pin");
  } else if (mode == "green") {
    analogWrite(rgbRedPin, 0);
    analogWrite(rgbGreenPin, 255);
    analogWrite(rgbBluePin, 0);
    html += "Testing with GREEN pin at PWM=255. LED should light green (COMMON CATHODE).";
    Serial.println("RGB TEST: Green test - writing PWM=255 to green pin");
  } else if (mode == "blue") {
    analogWrite(rgbRedPin, 0);
    analogWrite(rgbGreenPin, 0);
    analogWrite(rgbBluePin, 255);
    html += "Testing with BLUE pin at PWM=255. LED should light blue (COMMON CATHODE).";
    Serial.println("RGB TEST: Blue test - writing PWM=255 to blue pin");
  } else if (mode == "alloff") {
    // Common cathode: all at 0 to turn off
    analogWrite(rgbRedPin, 0);
    analogWrite(rgbGreenPin, 0);
    analogWrite(rgbBluePin, 0);
    html += "All pins at PWM=0 (common cathode OFF method). LED should be off now.";
    Serial.println("RGB TEST: All off test");
  } else {
    html += "No test selected yet.";
  }
  
  html += "</div>";
  html += "<p style='text-align:center;margin-top:20px;'><a href='/'>← Back to Control Panel</a></p>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Function to reset WiFi credentials and restart config portal
void handleWiFiReset() {
  if (!isAuthenticated()) return;
  
  String html = "<!DOCTYPE html><html><head><title>WiFi Reset</title>";
  html += "<style>body{font-family:Arial,sans-serif;padding:30px;background:#f0f0f0;}";
  html += ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;border-bottom:2px solid #f44336;padding-bottom:10px;}";
  html += ".info{background:#ffebee;padding:15px;border-radius:5px;margin:10px 0;border-left:4px solid #f44336;}";
  html += "</style></head><body><div class='container'>";
  html += "<h1>WiFi Reset</h1>";
  html += "<div class='info'><strong>Choose Mode:</strong><br><br>";
  html += "<p><a href='/wifireset?mode=ap' style='display:inline-block;padding:10px 20px;background:#4CAF50;color:white;text-decoration:none;border-radius:5px;'>Return to AP Mode</a></p>";
  html += "<p><a href='/wifireset?mode=config' style='display:inline-block;padding:10px 20px;background:#2196F3;color:white;text-decoration:none;border-radius:5px;'>Configure WiFi Network</a></p></div>";
  html += "</div></body></html>";
  
  String mode = server.arg("mode");
  
  if (mode == "ap") {
    // Just restart in AP mode
    server.send(200, "text/html", html);
    delay(2000);
    ESP.restart();
  } else if (mode == "config") {
    // Start WiFi configuration portal
    html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='2;url=http://192.168.4.1'>";
    html += "<style>body{font-family:Arial,sans-serif;padding:30px;background:#f0f0f0;}";
    html += ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "</style></head><body><div class='container'><h1>Starting WiFi Configuration...</h1>";
    html += "<p>Connecting to configuration portal...</p>";
    html += "<p>If not redirected, connect to WiFi: <strong>" + String(ap_name) + "</strong> and go to <strong>http://192.168.4.1</strong></p></div></body></html>";
    
    server.send(200, "text/html", html);
    Serial.println("Starting WiFi configuration portal...");
    
    delay(1000);
    
    // Stop server temporarily to start config portal
    server.stop();
    wifiManager.startConfigPortal(ap_name);
    
    // After portal closes, restart in STA mode
    ESP.restart();
  } else {
    server.send(200, "text/html", html);
  }
  
  Serial.println("WiFi reset requested");
}

// Function to switch from AP mode to connecting to a WiFi network
void handleConnectWiFi() {
  if (!isAuthenticated()) return;
  
  String html = "<!DOCTYPE html><html><head><title>Connect WiFi</title>";
  html += "<style>body{font-family:Arial,sans-serif;padding:30px;background:#f0f0f0;}";
  html += ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:10px;}";
  html += ".info{background:#fff3cd;padding:15px;border-radius:5px;margin:10px 0;border-left:4px solid #ffc107;}";
  html += "</style></head><body><div class='container'>";
  html += "<h1>Connect to WiFi Network</h1>";
  html += "<div class='info'><strong>⚠️ Restart Required</strong><br><br>";
  html += "To connect to your home WiFi network, please use 'Reset & Start AP Mode' below.<br><br>";
  html += "Then connect to the Ornament WiFi and configure your home network credentials.</div>";
  html += "<p style='text-align:center;margin-top:20px;'><a href='/wifistatus'>← Back to WiFi Status</a></p>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
  Serial.println("WiFi connection requested (requires restart)");
}

// Function to show current WiFi status and mode
void handleWiFiStatus() {
  if (!isAuthenticated()) return;
  
  String html = "<!DOCTYPE html><html><head><title>WiFi Status</title>";
  html += "<style>body{font-family:Arial,sans-serif;padding:30px;background:#f0f0f0;}";
  html += ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:10px;}";
  html += ".info{background:#e8f5e9;padding:15px;border-radius:5px;margin:10px 0;border-left:4px solid #4CAF50;}";
  html += ".button{padding:15px 30px;margin:10px;font-size:18px;cursor:pointer;border:none;border-radius:5px;color:white;text-decoration:none;display:inline-block;}";
  html += ".btn-primary{background:#2196F3;} .btn-secondary{background:#9C27B0;} .btn-danger{background:#f44336;}";
  html += "</style></head><body><div class='container'>";
  html += "<h1>WiFi Status</h1>";
  
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    html += "<div class='info'><strong>Current Mode:</strong> Access Point<br>";
    html += "<strong>SSID:</strong> " + String(ap_name) + "<br>";
    html += "<strong>IP Address:</strong> 192.168.4.1</div>";
    html += "<p><a href='/wificonnect' class='button btn-primary'>Connect to WiFi Network</a></p>";
  } else {
    html += "<div class='info'><strong>Current Mode:</strong> WiFi Client (STA)<br>";
    html += "<strong>SSID:</strong> " + String(WiFi.SSID()) + "<br>";
    html += "<strong>IP Address:</strong> " + ipAddress.toString() + "</div>";
  }
  
  html += "<p><a href='/wifireset' class='button btn-danger'>Reset & Start AP Mode</a></p>";
  html += "<p style='text-align:center;margin-top:20px;'><a href='/'>← Back to Control Panel</a></p>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}


void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  
  // Initialize LEDs to OFF (active HIGH: HIGH = ON, LOW = OFF)
  digitalWrite(ledPin1, LOW);  // Start with LEDs OFF
  digitalWrite(ledPin2, LOW);
  
  // Initialize RGB LED pins
  pinMode(rgbRedPin, OUTPUT);
  pinMode(rgbGreenPin, OUTPUT);
  pinMode(rgbBluePin, OUTPUT);
  
  // Initialize RGB LED to OFF (for common cathode LEDs, 0 = OFF)
  analogWrite(rgbRedPin, 0);
  analogWrite(rgbGreenPin, 0);
  analogWrite(rgbBluePin, 0);

  Serial.println();
  Serial.println("=== ESP8266 WiFi Setup ===");
  
  // Initialize EEPROM for WiFiManager
  EEPROM.begin(512);
  
  // FORCE portal mode - clears saved credentials (uncomment this line to force reset)
  // wifiManager.resetSettings();  // This will force the portal to appear
  
  // Configure WiFiManager
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  wifiManager.setConfigPortalTimeout(180);  // Portal timeout in seconds (3 minutes)
  
  // Add custom parameter to show IP after connection
  WiFiManagerParameter custom_text("<p>After connecting, your device will be available at its assigned IP address.</p>");
  wifiManager.addParameter(&custom_text);
  
  // Optional: Set callback for when config portal starts
  wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("\n🔧 Entered configuration mode");
    Serial.print("Access Point SSID: ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("Portal IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("\n📱 Connect your phone/laptop to WiFi:");
    Serial.println("   SSID: " + String(ap_name));
    Serial.println("   Open browser to: http://192.168.4.1");
    Serial.println("\n⏱️  You have 3 minutes to configure...");
  });
  
  // Set callback for when WiFi connects successfully
  wifiManager.setSaveConfigCallback([]() {
    Serial.println("\n✅ WiFi credentials saved!");
  });
  
  // Try to connect to saved WiFi credentials first
  Serial.println("Attempting to connect to WiFi...");
  
  // Try to auto-connect to saved WiFi credentials
  // If no credentials saved or connection fails, start configuration portal
  if (!wifiManager.autoConnect(ap_name)) {
    Serial.println("Failed to connect and hit timeout");
    Serial.println("Device will restart and try again...");
    delay(3000);
    ESP.restart();
  }
  
  // If we get here, WiFi connection was successful!
  Serial.println("");
  Serial.println("✅ WiFi connected!");
  Serial.println("================================");
  
  ipAddress = WiFi.localIP();
  currentSSID = WiFi.SSID();
  apMode = false;
  networkConnected = true;
  
  Serial.print("📍 Connect to: http://");
  Serial.print(ipAddress);
  Serial.println(":8080");
  Serial.print("📡 WiFi Network: ");
  Serial.println(currentSSID);
  Serial.print("Username: ");
  Serial.println(www_username);
  Serial.print("Password: ");
  Serial.println(www_password);
  Serial.println("================================");
  Serial.println("Server started on port 8080");

  // Start mDNS responder (optional - allows http://ornament.local:8080)
  if (MDNS.begin("ornament")) {
    Serial.println("✅ mDNS responder started at http://ornament.local:8080");
  } else {
    Serial.println("❌ Error setting up MDNS responder!");
  }

  // Set up URL handlers
  server.on("/", handleRoot);
  server.on("/led1/on", handleLED1On);
  server.on("/led1/off", handleLED1Off);
  server.on("/led1/fade", handleLED1Fade);  // Start/stop fade with ?action=start or ?action=stop
  server.on("/led1/fadespeed", handleLED1FadeSpeed);  // Set fade speed with ?speed=1-100
  server.on("/led2/on", handleLED2On);
  server.on("/led2/off", handleLED2Off);
  server.on("/both/on", handleBothOn);
  server.on("/both/off", handleBothOff);
  server.on("/rgb", handleRGBColor);  // Set RGB color with ?r=X&g=Y&b=Z
  server.on("/rgb/off", handleRGBOff);  // Turn RGB LED off
  server.on("/rgbtest", handleRGBTest);  // Test RGB LED type
  server.on("/info", handleInfo);  // Device information page
  server.on("/wifistatus", handleWiFiStatus);  // Show WiFi status and mode
  server.on("/wificonnect", handleConnectWiFi);  // Connect to WiFi network
  server.on("/wifireset", handleWiFiReset);  // Reset WiFi credentials
  
  // Start server
  server.begin();
  }

void loop() {
  server.handleClient();  // Handle web server requests
  MDNS.update();  // Handle mDNS requests
  
  // Handle LED1 fading effect
  if (led1FadeEnabled) {
    unsigned long currentTime = millis();
    if (currentTime - lastFadeUpdate >= led1FadeSpeed) {
      lastFadeUpdate = currentTime;
      
      // Update brightness
      led1Brightness += led1FadeDirection * 5;  // Step size of 5
      
      // Reverse direction at limits
      if (led1Brightness >= 255) {
        led1Brightness = 255;
        led1FadeDirection = -1;
      } else if (led1Brightness <= 0) {
        led1Brightness = 0;
        led1FadeDirection = 1;
      }
      
      // Write PWM value to LED
      analogWrite(ledPin1, led1Brightness);
    }
  }
}
