# WiFi Configuration Portal

Your ESP8266 now includes a WiFi configuration portal! No more hardcoding WiFi credentials.

## How It Works

### First Time Setup

1. **Power on the ESP8266**
   - If no WiFi credentials are saved, it will automatically create an Access Point (AP)

2. **Connect your phone/device to the AP:**
   - **SSID:** `ESP8266-Setup`
   - **Password:** None (open network)

3. **Open a web browser:**
   - Navigate to: `http://192.168.4.1`
   - A configuration page will appear automatically

4. **Enter your WiFi credentials:**
   - Select your home WiFi network from the list
   - Enter the WiFi password
   - Click "Save"

5. **Device reconnects:**
   - ESP8266 saves credentials and connects to your WiFi
   - It will use these credentials on every boot

### Subsequent Boots

- ESP8266 automatically connects using saved credentials
- No portal needed unless credentials are invalid or WiFi is unavailable

### Resetting WiFi Credentials

To change WiFi network or reset credentials:

1. **Via Web Interface:**
   - Open: `http://YOUR_ESP8266_IP:8080/wifireset`
   - Enter password: `admin` / `password`
   - Credentials will be cleared
   - Device will restart in configuration mode

2. **Via Serial:**
   - Monitor serial output for instructions
   - Portal appears if connection fails

## Configuration Portal Settings

- **AP Name:** `ESP8266-Setup`
- **AP IP:** `192.168.4.1`
- **Portal Timeout:** 3 minutes (180 seconds)
- **Credentials Storage:** EEPROM (persists after power cycle)

## Serial Monitor Output

When entering config mode:
```
=== ESP8266 WiFi Setup ===
Attempting to connect to WiFi...
Entered config mode
AP SSID: ESP8266-Setup
AP IP: 192.168.4.1

📱 Connect your phone to WiFi:
   SSID: ESP8266-Setup
   Open browser to: http://192.168.4.1

⏱️  You have 3 minutes to configure...
```

After successful connection:
```
✅ WiFi connected!
IP address: http://192.168.1.100
Server started on port 8080
```

## Benefits

✅ **No hardcoded credentials** - Change WiFi without reprogramming  
✅ **Easy setup** - Just connect phone and browse  
✅ **Works anywhere** - Configure once, use anywhere  
✅ **Persistent** - Credentials saved in EEPROM  
✅ **Auto-reconnect** - Connects automatically on boot  

## Troubleshooting

**Portal doesn't appear:**
- Check serial monitor for errors
- Make sure device isn't already connected to WiFi
- Reset credentials via `/wifireset` endpoint

**Can't connect to portal:**
- Make sure phone is connected to `ESP8266-Setup` network
- Try `http://192.168.4.1` directly (not https)
- Portal timeout is 3 minutes - reset device to restart

**After saving, device doesn't connect:**
- Check credentials are correct
- Verify WiFi network is in range
- Check serial monitor for connection errors
- Reset and try again

## Technical Details

- **Library:** WiFiManager v2.0.17
- **Storage:** EEPROM (512 bytes)
- **Fallback:** If portal times out, device restarts
- **Multiple Networks:** Portal shows available WiFi networks
- **Auto-detect:** Detects WPA, WPA2, WEP, Open networks

## Next Steps

1. **Upload the code** to your ESP8266
2. **First boot:** Connect phone to `ESP8266-Setup`
3. **Configure:** Enter WiFi credentials via browser
4. **Done!** Device will connect automatically

The configuration portal makes your ESP8266 truly portable - just set it up once and it will remember your WiFi!
