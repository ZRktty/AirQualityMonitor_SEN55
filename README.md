# SEN55 Air Quality Monitor

A comprehensive air quality monitoring system using ESP32-S3 and Sensirion SEN55 sensor with cloud logging, OTA updates, and real-time web dashboard.

![Project Status](https://img.shields.io/badge/status-active-success)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## 📋 Overview

This project creates an all-in-one air quality monitor that measures particulate matter (PM1.0, PM2.5, PM4, PM10), temperature, humidity, VOC (Volatile Organic Compounds), and NOx (Nitrogen Oxides) levels. Data is logged to ThingSpeak cloud platform with support for over-the-air (OTA) firmware updates and a local web dashboard for real-time monitoring.

## ✨ Features

### ✅ Implemented

- **Multi-Parameter Sensing**: 8 sensor readings from SEN55
  - PM1.0, PM2.5, PM4, PM10 (Particulate Matter in µg/m³)
  - Temperature (°C) and Humidity (%)
  - VOC Index (0-500) and NOx Index (0-500)
- **Data Averaging**: 20-sample moving average for stable readings
- **Cloud Logging**: Automatic upload to ThingSpeak every 20 seconds
- **OTA Updates**: Wireless firmware updates via Arduino IDE
- **Air Quality Classification**: PM2.5 levels categorized (Good/Moderate/Unhealthy)
- **Robust Error Handling**: Sensor validation, WiFi reconnection, upload retry logic
- **Non-blocking Architecture**: Efficient loop design for responsive OTA updates

### 🚧 In Development

- **Local Web Dashboard**: Real-time WebSocket-based visualization
  - WebSocket-only updates (no polling)
  - LittleFS storage for easy UI updates
  - Reusable gauge components (DRY principles)
  - Mobile-responsive design
  - See [Web Dashboard Plan](docs/plans/web-dashboard-feature.md)
- **MQTT Integration**: Home automation support
- **Display Support**: OLED/E-ink screen integration

### 📝 Planned

- **SD Card Logging**: Local data backup
- **Additional Sensors**: CO2, light, pressure
- **3D Printed Enclosure**: Custom housing design
- **Battery Power Option**: Portable deployment
- **AQI Calculations**: EPA Air Quality Index

## 📅 Development Plans

Active development plans and migration tasks:

- [x] **PlatformIO Migration**: Convert project to PlatformIO structure for better dependency management (see [Plan](docs/plans/platformio-migration.md))

- [ ] **Web Dashboard v1**: Basic WebSocket dashboard (see [Plan](docs/plans/web-dashboard-feature.md))

## 🛠 Hardware

### Current Components

| Component | Model | Purpose |
|-----------|-------|---------|
| **Microcontroller** | ESP32-S3-N16R8 WROOM | Main controller (16MB Flash, 8MB PSRAM) |
| **Air Quality Sensor** | Sensirion SEN55 | All-in-one environmental sensor |
| **Power Supply** | USB-C | 5V power input |

### Wiring Diagram

```
ESP32-S3               SEN55
---------             -------
GPIO1 (SDA) ------>   SDA
GPIO2 (SCL) ------>   SCL
5V          ------>   VDD
GND         ------>   GND
```

### Pin Configuration

- **I2C SDA**: GPIO1
- **I2C SCL**: GPIO2

## 📦 Software Dependencies

### Arduino Libraries

Install via Arduino IDE Library Manager:

```
- SensirionI2CSen5x (Sensirion sensor driver)
- WiFi (ESP32 built-in)
- HTTPClient (ESP32 built-in)
- ArduinoOTA (ESP32 built-in)
- Wire (I2C communication)
- ESPAsyncWebServer by me-no-dev (web dashboard)
- AsyncTCP by me-no-dev (dependency for web server)
- ArduinoJson by Benoit Blanchon (JSON serialization)
- LittleFS (ESP32 built-in, filesystem for web assets)
```

### Board Support

- **Board**: ESP32-S3 Dev Module
- **ESP32 Core**: v2.0.0 or later
- **Board URL**: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

## ⚙️ Configuration

### 1. Create Configuration File

Copy the example configuration:

```bash
cp config.example.h config.h
```

### 2. Edit Configuration

Open `config.h` and fill in your credentials:

```cpp
// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ThingSpeak settings
const char* THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_API_KEY";
const unsigned long THINGSPEAK_CHANNEL_ID = YOUR_CHANNEL_ID;

// OTA settings
const char* OTA_HOSTNAME = "SEN55-AirQuality";
const char* OTA_PASSWORD = "YOUR_OTA_PASSWORD";

// Web Dashboard
// Accessible at http://<device-ip>/ or http://sen55-airquality.local/
// Real-time updates via WebSocket (no configuration needed)
```

### 3. ThingSpeak Setup

1. Create free account at [ThingSpeak.com](https://thingspeak.com)
2. Create new channel with 8 fields:
   - Field 1: PM1.0
   - Field 2: PM2.5
   - Field 3: PM4
   - Field 4: PM10
   - Field 5: Temperature
   - Field 6: VOC Index
   - Field 7: NOx Index
   - Field 8: Humidity
3. Copy your **Write API Key** and **Channel ID** to `config.h`

## 🚀 Installation

### Initial Upload (USB)

1. Connect ESP32-S3 to computer via USB-C
2. Open `AirQualityMonitor_SEN55.ino` in Arduino IDE
3. Select **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module**
4. Select correct **Port** (e.g., `/dev/cu.usbserial-*` on Mac)
5. Click **Upload** button
6. Open **Serial Monitor** (115200 baud) to view output

### OTA Updates (Wireless)

After initial upload, you can update firmware over WiFi:

1. Wait for device to connect to WiFi (check Serial Monitor for IP)
2. In Arduino IDE, go to **Tools → Port**
3. Select **Network ports → SEN55-AirQuality at [IP_ADDRESS]**
4. Enter OTA password when prompted
5. Upload as normal

## 📊 Usage

### Serial Monitor Output

```
================================
=== SEN55 ThingSpeak Logger ===
================================

Connecting to WiFi: YourNetwork
...
WiFi connected!
IP address: 192.168.1.100
ThingSpeak Channel: 3166664

✓ OTA Ready!
  Hostname: SEN55-AirQuality
  IP: 192.168.1.100
  Upload via: Tools → Port → Network ports

🌐 Web Dashboard: http://192.168.1.100/
📱 Mobile Access: http://sen55-airquality.local/

================================

PM1.0:8.2 | PM2.5:12.5 µg/m³ 🟢 [GOOD] | PM4:15.1 | PM10:18.3 | Temp:22.3°C | Hum:45.2% | VOC:120 | NOx:85 | Avg:15/20 | Upload in 5s
```

### Air Quality Levels (PM2.5)

| Level | Range (µg/m³) | Indicator | Description |
|-------|---------------|-----------|-------------|
| 🟢 Good | 0 - 15 | Green | Air quality is satisfactory |
| 🟡 Moderate | 15 - 35 | Yellow | Acceptable for most people |
| 🟠 Unhealthy (Sensitive) | 35 - 55 | Orange | May affect sensitive groups |
| 🔴 Unhealthy | 55+ | Red | Health effects for everyone |

### Data Upload Behavior

- **Sensor Reading**: Every 1 second
- **Data Averaging**: 20 samples (rolling average)
- **Upload Interval**: Every 20 seconds
- **Upload Retry**: Data preserved on failure for next interval
- **ThingSpeak Limit**: 15-second minimum (free tier)

### Web Dashboard Access

Once the device is connected to WiFi, access the dashboard:

**By IP Address:**

- Open browser to `http://<device-ip>/` (see Serial Monitor for IP)
- Example: `http://192.168.1.100/`

**By Hostname (mDNS):**

- Open browser to `http://sen55-airquality.local/`
- Works on macOS/iOS natively
- Windows requires Bonjour service

**Dashboard Features:**

- Real-time sensor readings (1 Hz updates via WebSocket)
- Color-coded PM2.5 gauge with AQI zones
- Individual gauges for VOC and NOx indices
- Temperature, humidity, and particulate matter cards
- 1-minute history sparkline charts
- System status (WiFi signal, uptime, connection)
- Mobile-responsive design

**Technical Details:**

- WebSocket-only updates (no REST polling)
- Sub-second latency
- Automatic reconnection on disconnect
- LittleFS storage for web assets
- Reusable gauge components

## 🏗 Project Structure

```
AirQualityMonitor_SEN55/
├── AirQualityMonitor_SEN55.ino  # Main program
├── DataAveraging.cpp/h          # Moving average calculation
├── SensorUtils.cpp/h            # Sensor utilities and validation
├── config.h                     # Local configuration (gitignored)
├── config.example.h             # Configuration template
├── data/                        # LittleFS web assets (upload to device)
│   ├── index.html              # Dashboard HTML
│   ├── style.css               # Dashboard styles
│   ├── script.js               # WebSocket client & UI logic
│   └── favicon.ico             # Browser icon
├── docs/
│   └── plans/
│       └── web-dashboard-feature.md  # Feature planning docs
├── datasheets/
│   └── Sensirion_Datasheet_SEN5x.pdf
└── README.md                    # This file
```

## 🔧 Troubleshooting

### Sensor Not Detected

```
ERROR reading sensor: Error trying to execute operation
⚠️  Check wiring! Skipping this reading...
```

**Solution:**

- Verify I2C wiring (SDA → GPIO1, SCL → GPIO2)
- Check power connections (5V and GND)
- Ensure pull-up resistors (usually integrated on SEN55)

### WiFi Connection Issues

**Solution:**

- Verify SSID and password in `config.h`
- Check 2.4GHz WiFi availability (ESP32 doesn't support 5GHz)
- Move closer to WiFi router

### ThingSpeak Upload Failures

```
✗ Upload failed - check API key or rate limit
```

**Solution:**

- Verify Write API Key in `config.h`
- Check Channel ID matches your ThingSpeak channel
- Ensure 15+ second intervals between uploads (free tier limit)
- Verify internet connectivity

### OTA Update Not Appearing

**Solution:**

- Ensure device is connected to WiFi
- Check that device and computer are on same network
- Verify OTA hostname in **Tools → Port → Network ports**
- Wait 30 seconds after device boots for OTA to initialize

### Web Dashboard Not Loading

**Solution:**

- Verify device IP address in Serial Monitor
- Ensure device and computer are on same WiFi network
- Try IP address if mDNS hostname doesn't work
- Check that LittleFS files were uploaded (Tools → ESP32 Sketch Data Upload)
- Clear browser cache and reload page

### WebSocket Connection Failing

```
Connection Error - Reconnecting...
```

**Solution:**

- Check browser console for errors (F12 → Console)
- Verify WebSocket port not blocked by firewall
- Try different browser (Chrome/Firefox recommended)
- Restart device and refresh browser page

## 🔐 Security Notes

- `config.h` is gitignored to protect credentials
- Never commit API keys or passwords to version control
- Change default OTA password for production deployments
- Consider using HTTPS for ThingSpeak (requires code modification)

## 📈 Future Enhancements

See [docs/plans/](docs/plans/) for detailed feature planning:

- **Web Dashboard v2.0**:
  - Configuration page (WiFi, ThingSpeak, calibration)
  - 24-hour trend graphs
  - AQI calculations (EPA standard)
  - CSV data export
  - Authentication/security
- **MQTT Integration**: Home Assistant / Node-RED support
- **Historical Data**: SD card logging and export
- **Multi-Device Support**: Network of monitors
- **Battery Operation**: Deep sleep power optimization
- **Notifications**: Threshold alerts via email/Telegram

## 🤝 Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [Sensirion](https://www.sensirion.com/) for SEN55 sensor and excellent documentation
- [ThingSpeak](https://thingspeak.com/) for free IoT data platform
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) community
- Arduino and maker community for inspiration

## 📞 Contact

**Project Maintainer**: ZRktty  
**Repository**: [github.com/ZRktty/AirQualityMonitor_SEN55](https://github.com/ZRktty/AirQualityMonitor_SEN55)

---

**Built with ❤️ for cleaner air monitoring**
