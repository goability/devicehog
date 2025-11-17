# Local Usage Instructions for hogcloud

## Overview
This ESP8266 device runs **only locally** on your network. No external API connections are made.

## First-Time Setup

### Step 1: Power on the device
When you first power on the device, it will create a WiFi access point called **"hogcloud"**.

### Step 2: Connect to the device
1. Look for a WiFi network called **"hogcloud"**
2. Connect your phone/laptop to this network (no password required)
3. Open your web browser and go to: `http://192.168.4.1`

### Step 3: Configure WiFi
1. You'll see a configuration page showing available WiFi networks
2. Select YOUR home WiFi network
3. Enter your WiFi password
4. Click "Save"

### Step 4: Wait for connection
- The device will connect to your WiFi
- The "hogcloud" access point will disappear
- The device will restart

## Accessing the Device (After Setup)

Once configured, you need to find the device's IP address. Here are your options:

### Option 1: Use mDNS (easiest - if you're on the same WiFi)
**Simply go to:** `http://hogcloud.local:8080`

### Option 2: Check Serial Monitor (if available)
If you have the device connected to your computer via USB:
```bash
./bin/arduino-cli monitor -p /dev/cu.SLAB_USBtoUART --config baudrate=115200
```
Look for the output showing:
```
📍 Connect to: http://192.168.1.XXX:8080
```

### Option 3: Use Device Info Page
1. First, you need to access the device once (try `http://hogcloud.local:8080` or scan your network)
2. Once you're connected, click the **"📡 Device Info"** button
3. This will show you all the connection details including the IP address

### Option 4: Check Your Router
1. Connect to your router's admin panel (usually `http://192.168.1.1`)
2. Look for "Connected Devices" or "DHCP Client List"
3. Find "hogcloud" in the list
4. Note the IP address shown

### Option 5: Network Scanner App
Use a network scanner app on your phone:
- **iOS**: "Network Analyzer" or "Fing"
- **Android**: "Fing" or "Network IP Scanner"
- These show all connected devices on your network

## Using the Device

Once you know the IP address:

### Web Interface
- **URL**: `http://[IP_ADDRESS]:8080` or `http://hogcloud.local:8080`
- **Username**: `admin`
- **Password**: `password`

### Features
- **LED Control**: Turn individual LEDs or both LEDs ON/OFF
- **Device Info**: Click "📡 Device Info" to see IP address and connection details
- **Reset WiFi**: Click "Reset WiFi" to clear saved credentials and restart the config portal

## Troubleshooting

### Can't find the device?
1. **Try mDNS**: `http://hogcloud.local:8080`
2. **Check your WiFi**: Make sure you're on the same network
3. **Reset WiFi**: Use the "Reset WiFi" button on the web interface, then reconnect to "hogcloud" AP

### Forgot the IP address?
- Use any of the options listed above to find it again
- The "Device Info" page on the web interface will always show it

### Want to connect to a different WiFi?
- Click "Reset WiFi" on the web interface
- The device will restart in configuration mode
- Connect to "hogcloud" AP and go through setup again

## Security Note

⚠️ **Important**: The default password is "password". Consider changing this in the code before deploying if security is a concern.
