# Christmas Ornament LED Controller

This device lets you control the LED lights on your Christmas ornament from your phone, tablet, or computer.

## Getting Started

### Step 1: Power On the Device

Plug in the device to power it on. Wait a few seconds for it to start up.

### Step 2: Connect to the Device

1. On your phone, tablet, or computer, open your WiFi settings
2. Look for a WiFi network named **"Ornament"**
3. Connect to this network (no password needed)
4. Once connected, open a web browser and go to: **http://192.168.4.1:8080**
5. When prompted, enter:
   - **Username:** admin
   - **Password:** password

You should now see the Ornament Control Panel!

## Controlling the Lights

### Change Colors

- Use the color picker on the control panel to choose any color you want
- Click the color you want and the lights will change immediately
- You can pick from millions of colors!

### Turn Lights On/Off

- Click the **"Lights OFF"** button to turn the lights off
- Use the color picker again to turn them back on with your chosen color

## Connecting to Your Home WiFi Network

To control the device from anywhere in your home (without connecting to the "Ornament" network), you can connect it to your home WiFi:

1. While connected to the "Ornament" network, go to the control panel
2. Click the **"WiFi Settings"** button
3. Click **"Reset & Start AP Mode"**
4. Click **"Configure WiFi Network"**
5. A configuration page will appear - select your home WiFi network from the list
6. Enter your home WiFi password
7. The device will restart and connect to your home network

**Important:** Once the device connects to your home WiFi, the "Ornament" WiFi network will disappear. The device can only be in one mode at a time - either Access Point mode (showing "Ornament" network) or connected to your home WiFi. You cannot connect to the "Ornament" network after it's connected to your home WiFi.

## Accessing the Device on Your Home Network

Once the device is connected to your home WiFi:

1. Make sure your phone, tablet, or computer is connected to the same home WiFi network
2. **Find the device's IP address:**
   - **Option 1:** Check your router's admin page to see connected devices and look for "Ornament" or an ESP8266 device
   - **Option 2:** Use a network scanner app on your phone to find devices on your network
   - **Option 3:** Try common IP addresses (see "Finding the Device on Your Network" section below)
3. Open a web browser and go to: **http://[IP_ADDRESS]:8080** (replace [IP_ADDRESS] with the actual IP address, for example: http://192.168.1.100:8080)
4. Enter your username (admin) and password (password)
5. You can now control the lights from anywhere in your home!

**Note:** Some devices may also work with **http://ornament.local:8080**, but this doesn't work on all networks. Using the IP address is more reliable.

**To get back to "Ornament" AP mode:** If you need to access the device via the "Ornament" network again, you can use the "Reset & Start AP Mode" button from the web interface (if you can access it), or perform a factory reset by holding the RESET button for 5 seconds.

## Finding the Device on Your Network

If you can't connect to the device on your home network, here are several ways to find it:

### Method 1: Check if Device is Connected (Easiest)

First, verify the device is actually connected to your WiFi:

1. **Check if "Ornament" WiFi network appears:**
   - If you see the "Ornament" network, the device is in Access Point mode (not connected to your home WiFi)
   - If you DON'T see "Ornament" network, the device is likely connected to your home WiFi (the AP disappears when connected to home network)

2. **If "Ornament" network is visible:**
   - Connect to it and go to http://192.168.4.1:8080
   - Click "Device Info" to see the current status
   - If it shows it's connected to your home WiFi, note the IP address shown
   - If it's not connected, use "WiFi Settings" to configure your home network

### Method 2: Scan Your Network (Mac/Linux)

Open Terminal and run these commands to scan your network:

1. **Find your network range:**
   ```bash
   ifconfig | grep "inet " | grep -v 127.0.0.1
   ```
   This shows your IP address (e.g., `192.168.1.50` means your network is `192.168.1.x`)

2. **Scan for devices on port 8080:**
   ```bash
   # Replace 192.168.1 with your network range from step 1
   for i in {1..254}; do
     timeout 1 bash -c "echo >/dev/tcp/192.168.1.$i/8080" 2>/dev/null && echo "Found device at 192.168.1.$i:8080"
   done
   ```

3. **Or use nmap (if installed):**
   ```bash
   # Install nmap first: brew install nmap
   # Replace 192.168.1.0/24 with your network range
   nmap -p 8080 --open 192.168.1.0/24
   ```

### Method 3: Check Your Router

1. Open your router's admin page (usually `http://192.168.1.1` or `http://192.168.0.1`)
2. Log in with your router password
3. Look for "Connected Devices", "DHCP Clients", or "Network Map"
4. Find a device named "Ornament" or look for an ESP8266 device
5. Note the IP address shown

### Method 4: Use Network Scanner App

**On iPhone/iPad:**
- Install "Fing" or "Network Analyzer" from the App Store
- Open the app and scan your network
- Look for "Ornament" or unknown devices

**On Android:**
- Install "Fing" or "Network IP Scanner"
- Scan your network and look for the device

### Method 5: Try Common IP Addresses

If you know your network range, try these common addresses in your browser:
- `http://192.168.1.100:8080`
- `http://192.168.1.101:8080`
- `http://192.168.1.102:8080`
- `http://192.168.0.100:8080`
- `http://192.168.0.101:8080`

(Replace with your actual network range - check your computer's IP address to know your range)

### Method 6: Reset and Reconnect

If you still can't find it:

1. Press and hold the RESET button on the device for 5 seconds
2. Wait for the "Ornament" network to appear
3. Connect to "Ornament" network
4. Go to http://192.168.4.1:8080
5. Click "WiFi Settings" → "Reset & Start AP Mode" → "Configure WiFi Network"
6. Reconnect to your home WiFi
7. After it connects, click "Device Info" to get the new IP address

## Troubleshooting WiFi Connection Issues

If the device is not connecting to your home WiFi network, follow these steps:

### Step 1: Verify You Can Access the Device
1. Check if the "Ornament" WiFi network is visible
2. If visible, connect to it and go to http://192.168.4.1:8080
3. Log in with username: `admin` and password: `password`
4. Click "Device Info" to see current status

### Step 2: Check WiFi Configuration Process
1. From the control panel, click "WiFi Settings"
2. Click "Reset & Start AP Mode"
3. Click "Configure WiFi Network"
4. You should see a configuration page with available WiFi networks
5. **Important:** Make sure you:
   - Select the correct WiFi network name (SSID)
   - Enter the password correctly (WiFi passwords are case-sensitive)
   - Wait for the configuration to complete (up to 3 minutes)

### Step 3: Common Issues and Solutions

**Issue: Configuration portal doesn't appear**
- Make sure you clicked "Configure WiFi Network" and wait a few seconds
- The device may redirect you - check the URL in your browser
- Try manually going to http://192.168.4.1 (without :8080) when connected to "Ornament" network

**Issue: Page sits there after selecting WiFi network and entering password**
- **What should happen:** After entering your WiFi password and clicking "Save", the page should show a "Connecting..." message, then either:
  - Success: Show a success message with the device's new IP address, or redirect you
  - Failure: Show an error message or the page may timeout
- **If the page just sits there:**
  - Wait 30-60 seconds for the connection attempt (this can take time)
  - Check if you entered the correct WiFi password (case-sensitive!)
  - The connection might be failing silently
  - After 3 minutes, the portal will timeout and the device will restart
  - If "Ornament" network reappears, the connection failed - try again with the correct password

**Issue: Can't see your WiFi network in the list**
- Your WiFi network might be hidden (not broadcasting SSID)
- Try moving the device closer to your router
- Some 5GHz-only networks may not appear - make sure your router has 2.4GHz enabled (ESP8266 only supports 2.4GHz)

**Issue: Connection fails after entering password**
- Double-check the WiFi password is correct (case-sensitive)
- Make sure your router isn't blocking new devices
- Check if your router has MAC address filtering enabled
- Try restarting your router

**Issue: Device connects but then "Ornament" network reappears**
- This means the connection attempt failed and the device restarted back to AP mode
- **Most common causes:**
  - **Wrong WiFi password** - Double-check it's exactly correct (case-sensitive)
  - **Router blocking the device** - Check router settings for MAC filtering or device blocking
  - **Weak signal** - Move device closer to router during setup
  - **Router not accepting new connections** - Try restarting your router
  - **Network incompatibility** - Make sure you selected the 2.4GHz network (not 5GHz)
- **What to do:**
  - Try the connection process again, being very careful with the password
  - Make sure you're selecting the correct network name (SSID)
  - Try moving the device right next to your router during setup
  - Check your router's admin page to see if it's blocking the connection

**Issue: Device connects but you can't find it on your network**
- Wait 1-2 minutes after connection for the device to fully start
- Check your router's connected devices list
- Try scanning your network (see "Finding the Device on Your Network" section)
- The device may have gotten a different IP address than expected

### Step 4: Factory Reset and Retry
If nothing works:
1. Perform a factory reset (hold RESET button for 5 seconds)
2. Wait for "Ornament" network to appear
3. Connect and go through the WiFi setup process again
4. Make sure you're very close to your router during setup

## General Troubleshooting

- **Can't find the "Ornament" network?** Make sure the device is powered on and wait a minute for it to start up
- **Can't access the control panel?** Make sure you're connected to the "Ornament" WiFi network and using the correct address: http://192.168.4.1:8080
- **Can't connect to ornament.local:8080?** This address doesn't work on all networks. Use the device's IP address instead (see "Finding the Device on Your Network" section above)
- **Device not showing up on network?** The device might not be connected to your WiFi. Connect to "Ornament" network and check "Device Info" to see connection status

## Factory Reset

If you need to reset the device back to factory settings (clearing all saved WiFi passwords and settings):

1. Locate the **RESET** button on the device
2. Press and hold the reset button for about 5 seconds
3. Release the button
4. The device will restart and return to factory settings
5. After restarting, the device will create the "Ornament" WiFi network again
6. Connect to the "Ornament" network and access the control panel at **http://192.168.4.1:8080**

**Note:** This will erase any saved WiFi network passwords. You'll need to set up your home WiFi connection again if you want to use the device on your home network.

