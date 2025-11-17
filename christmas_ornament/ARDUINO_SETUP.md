# Arduino IDE Setup Instructions

This guide will help you set up the Arduino IDE to compile and upload the ESP8266 code.

## Step 1: Install Arduino IDE

Download and install Arduino IDE from: https://www.arduino.cc/en/software

## Step 2: Add ESP8266 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In the **Additional Boards Manager URLs** field, add:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Click **OK**

5. Go to **Tools → Board → Boards Manager**
6. Search for **"esp8266"**
7. Install **"esp8266" by ESP8266 Community** (version 3.x.x recommended)
8. Wait for installation to complete (this may take a few minutes)

## Step 3: Install Required Libraries

You need to install the following libraries:

### Method 1: Install via Library Manager (Easiest)

1. Go to **Tools → Manage Libraries**
2. Install the following libraries one by one:

   **WiFiManager**
   - Search for **"WiFiManager"**
   - Install **"WiFiManager by tzapu"**

   **Note:** Most other libraries (ESP8266WiFi, ESP8266WebServer, ESP8266mDNS, DNSServer, EEPROM) are included with the ESP8266 board package you installed in Step 2.

## Step 4: Select Your Board

1. Go to **Tools → Board → ESP8266 Boards**
2. Select the appropriate board for your device:
   - **For NodeMCU:** Select **"NodeMCU 1.0 (ESP-12E Module)"**
   - **For D1 Mini ESP-12F:** Select **"WeMos D1 R2 & mini"**

## Step 5: Configure Board Settings

1. Go to **Tools** menu and configure:
   - **Upload Speed**: 115200
   - **CPU Frequency**: 80 MHz
   - **Flash Size**: 4MB (FS:2MB OTA:~1019KB)
   - **Debug Level**: None
   - **Port**: Select the COM port where your ESP8266 is connected (e.g., /dev/cu.SLAB_USBtoUART on Mac, COM3 on Windows)

## Step 6: Open Your Sketch

1. Open `nodemcu/nodemcu.ino` in Arduino IDE

## Step 7: Verify and Upload

1. Click the **Verify** button (checkmark icon) to compile the code
2. If compilation succeeds, click **Upload** button (arrow icon)
3. Wait for the upload to complete

## Troubleshooting

### Library Not Found Errors

If you see errors like "cannot open source file 'WiFiManager.h'":
- Make sure you installed WiFiManager from the Library Manager
- Go to **Sketch → Include Library → Manage Libraries** and verify WiFiManager is installed

### ESP8266 Board Not Found

If you can't find ESP8266 boards in the board list:
- Make sure you added the ESP8266 board manager URL correctly
- Try restarting Arduino IDE
- Go to **Tools → Board → Boards Manager** and verify ESP8266 is installed

### Port Not Available

If you don't see your ESP8266 in the port list:
- Make sure the ESP8266 is connected via USB
- Install the USB-to-Serial drivers for your board:
  - **CH340**: Common on D1 Mini boards (most common)
  - **CP2102**: Used on some NodeMCU boards
- Try unplugging and replugging the USB cable
- On Mac, you may need to grant permissions to the USB-to-Serial driver

### Upload Fails

If upload fails:
- Hold the BOOT button on the ESP8266 while clicking Upload
- Try different USB cable (some cables are charge-only)
- Lower the upload speed in Tools menu
- Make sure you selected the correct board

## Required Libraries Summary

✅ **Included with ESP8266 Board Package:**
- ESP8266WiFi
- ESP8266WebServer  
- ESP8266mDNS
- DNSServer
- EEPROM

✅ **Install via Library Manager:**
- WiFiManager (by tzapu)

## Next Steps

After successfully uploading the code, follow the instructions in `LOCAL_USAGE_INSTRUCTIONS.md` to configure WiFi and use the device.
