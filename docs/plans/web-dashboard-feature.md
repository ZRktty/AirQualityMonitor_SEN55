# Local Web Dashboard Feature Plan

**Status:** Planning  
**Priority:** High  
**Estimated Effort:** 2-3 days  
**Target:** ESP32-S3 SEN55 Air Quality Monitor

---

## Overview

Add a real-time web dashboard accessible via the ESP32's IP address, allowing users to monitor air quality readings without relying on ThingSpeak. The dashboard will display live sensor data, historical trends, and system status with automatic updates.

## Current State Analysis

### Available Sensor Data

- **PM1.0, PM2.5, PM4, PM10** - Particulate Matter (µg/m³)
- **Temperature** - °C
- **Humidity** - %
- **VOC Index** - Volatile Organic Compounds (0-500)
- **NOx Index** - Nitrogen Oxides (0-500)

### Existing Infrastructure

- ✅ WiFi connectivity established
- ✅ ESP32-S3 with 512KB SRAM, plenty of flash
- ✅ Non-blocking sensor loop (1-second intervals)
- ✅ Data averaging system (20 samples)
- ✅ OTA update system (proves network accessibility)
- ✅ Data validation and quality checks
- ❌ No web server currently implemented
- ❌ No HTTP endpoints

### Available Resources

- **GPIO Pins:** GPIO3-21, GPIO33-45 available (GPIO1/2 used for I2C)
- **Memory:** ~400-450 KB SRAM free
- **Flash:** 2-4 MB free (sufficient for web assets)

---

## Implementation Steps

### Step 1: Add Async Web Server Library and Setup Endpoints

**Dependencies:**

```cpp
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
```

**Libraries to Install:**

- `ESPAsyncWebServer` by me-no-dev
- `AsyncTCP` by me-no-dev (dependency)
- `ArduinoJson` by Benoit Blanchon

**REST API Endpoints:**

| Endpoint | Method | Description | Response |
|----------|--------|-------------|----------|
| `/` | GET | Serve dashboard HTML | HTML page |
| `/api/current` | GET | Latest sensor readings | JSON |
| `/api/average` | GET | 20-sample averaged data | JSON |
| `/api/status` | GET | System info (WiFi, uptime, sensor health) | JSON |

**JSON Response Format:**

```json
{
  "pm1": 12.3,
  "pm25": 15.7,
  "pm4": 18.2,
  "pm10": 20.5,
  "temperature": 22.5,
  "humidity": 45.3,
  "voc": 125,
  "nox": 89,
  "timestamp": 1234567890,
  "quality": "GOOD"
}
```

**Integration Points:**

- Initialize `AsyncWebServer` on port 80 in `setup()`
- Place server initialization after OTA setup
- Use existing sensor reading variables (no new globals needed)
- Non-blocking operation - no impact on sensor loop

---

### Step 2: Create Responsive HTML Dashboard with Live Data Display

**Dashboard Features:**

1. **PM2.5 Gauge Widget**
   - Large circular gauge with color-coded AQI zones
   - Green (0-15), Yellow (15-35), Orange (35-55), Red (55+)
   - Animated needle with real-time updates

2. **Sensor Cards Grid**
   - 8 cards displaying all sensor readings
   - Icons for each parameter
   - Color-coded status indicators
   - Units clearly labeled

3. **Mini Trend Sparklines**
   - Last 60 readings (1 minute of history)
   - Inline with each sensor card
   - Lightweight SVG-based visualization

4. **System Status Bar**
   - WiFi signal strength
   - Device uptime
   - Last update timestamp
   - Connection status indicator

**Technical Approach:**

**Option A: PROGMEM Embedded HTML (Recommended for MVP)**

- Minified HTML/CSS/JS embedded in flash memory
- ~10-15KB total size
- No filesystem required
- Faster initial load
- Best for static dashboard

**Option B: LittleFS Filesystem**

- HTML/CSS/JS stored as separate files
- Can update UI via OTA without code changes
- Better for development/frequent UI iterations
- Requires partition table modification

**Recommendation:** Start with PROGMEM for simplicity, migrate to LittleFS in v2 if UI updates become frequent.

**Data Update Strategy:**

```javascript
// Poll /api/current every 2 seconds
setInterval(async () => {
  const data = await fetch('/api/current').then(r => r.json());
  updateDashboard(data);
}, 2000);
```

**Styling:**

- Mobile-first responsive design
- CSS Grid for layout
- Minimal dependencies (no frameworks)
- Dark theme for better visibility
- < 5KB total CSS

---

### Step 3: Add WebSocket for Real-Time Push Updates

**Why WebSockets?**

- ✅ More efficient than REST polling (no HTTP overhead)
- ✅ Sub-second latency (updates every 1 second)
- ✅ Lower bandwidth usage
- ✅ Bidirectional communication for future features

**Implementation:**

**Server Side:**

```cpp
AsyncWebSocket ws("/ws");

void setup() {
  // ... existing code ...
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  // ... existing sensor reading code ...
  
  // Broadcast to all connected clients
  String json = createSensorJSON();
  ws.textAll(json);
}
```

**Client Side:**

```javascript
const ws = new WebSocket(`ws://${window.location.hostname}/ws`);

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  updateDashboard(data);
};

ws.onerror = () => {
  // Fallback to REST polling
  startPolling();
};
```

**Benefits:**

- Real-time updates synchronized with sensor readings (1 Hz)
- Automatic fallback to REST if WebSocket fails
- Connection status visible to user
- Minimal code changes to existing loop

---

### Step 4: Integrate Dashboard Link in Serial Output

**Serial Output Enhancements:**

```
================================
=== SEN55 ThingSpeak Logger ===
================================

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
```

**mDNS Responder:**

```cpp
#include <ESPmDNS.h>

void setup() {
  // ... after WiFi connection ...
  if (MDNS.begin("sen55-airquality")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started: http://sen55-airquality.local/");
  }
}
```

**Benefits:**

- Friendly hostname (no need to remember IP)
- Works on same local network
- Compatible with macOS/iOS (native mDNS support)
- Windows requires Bonjour service

---

## Further Considerations

### 1. Storage Method Decision

**PROGMEM (Recommended for v1.0)**

- ✅ Simpler implementation
- ✅ No filesystem overhead
- ✅ Faster boot time
- ❌ Requires recompilation for UI changes
- Implementation: Store HTML as `const char PROGMEM htmlPage[] = "...";`

**LittleFS (Recommended for v2.0+)**

- ✅ Update UI via OTA without code changes
- ✅ Easier development workflow
- ✅ Can serve larger files
- ❌ Requires partition table setup
- ❌ Slightly slower file serving

**Decision:** Use PROGMEM for MVP, document LittleFS migration path.

---

### 2. Historical Data Caching

**Circular Buffer Approach:**

```cpp
#define HISTORY_SIZE 60  // 1 minute at 1Hz

struct SensorReading {
  float pm25;
  float temperature;
  float humidity;
  unsigned long timestamp;
};

SensorReading history[HISTORY_SIZE];
uint8_t historyIndex = 0;
```

**Memory Cost:** 60 readings × 16 bytes = ~1KB RAM

**API Endpoint:** `/api/history` returns last 60 readings as JSON array

**Client Rendering:** Lightweight sparkline charts using SVG or Canvas

**Alternative:** Fetch historical data from ThingSpeak API if internet available (no RAM cost, but slower)

**Recommendation:** Implement circular buffer (minimal RAM cost, always available)

---

### 3. Configuration Page Feature

**Potential `/config` Page Features:**

- WiFi SSID/Password
- ThingSpeak API Key/Channel ID
- OTA Password
- Sensor calibration offsets
- Upload interval settings
- Display preferences (units, thresholds)

**Requirements:**

- Form handling endpoints (`/api/config` POST)
- `Preferences` library for ESP32 NVS storage
- Authentication (password-protected)
- Backup/restore configuration

**Complexity:** Medium (2-3 days additional work)

**Recommendation:** Defer to v2.0 - not critical for MVP dashboard

---

### 4. Additional Future Features

**Authentication & Security:**

- Basic Auth or session-based login
- Secure credentials in NVS
- HTTPS support (requires certificate management)

**Advanced Visualizations:**

- 24-hour trend graphs (requires more history storage)
- AQI calculations (EPA standard)
- Export data as CSV
- Print-friendly report view

**Notifications:**

- Threshold alerts (visual warnings on dashboard)
- Future: Email/Telegram integration

**Multi-device Support:**

- Multiple ESP32 units reporting to single dashboard
- Requires MQTT or central server

---

## Success Criteria

- ✅ Dashboard accessible via `http://<IP>/` on local network
- ✅ All 8 sensor readings displayed and updated in real-time
- ✅ PM2.5 AQI gauge with color-coded zones
- ✅ Mobile-responsive design (works on phone/tablet)
- ✅ WebSocket connection with REST polling fallback
- ✅ No impact on existing ThingSpeak upload or OTA functionality
- ✅ Dashboard link printed in serial output
- ✅ mDNS hostname working (`sen55-airquality.local`)

---

## Implementation Timeline

**Phase 1: Basic Web Server (Day 1)**

- Install libraries
- Add REST API endpoints
- Test JSON responses
- Verify no impact on sensor loop

**Phase 2: Dashboard UI (Day 2)**

- Create HTML/CSS layout
- Implement REST polling
- Add sensor cards and gauges
- Test responsive design

**Phase 3: WebSocket Integration (Day 2-3)**

- Add WebSocket server
- Implement client-side WebSocket
- Add connection status indicators
- Test fallback behavior

**Phase 4: Polish & Testing (Day 3)**

- Add mDNS responder
- Update serial output
- Test on multiple devices
- Documentation updates

---

## Testing Plan

1. **Unit Testing:**
   - JSON serialization correctness
   - API endpoint responses
   - WebSocket message format

2. **Integration Testing:**
   - Concurrent ThingSpeak uploads + web serving
   - OTA updates while dashboard active
   - Multiple client connections

3. **Performance Testing:**
   - Memory usage monitoring
   - CPU load during web serving
   - WebSocket broadcast timing

4. **Compatibility Testing:**
   - Chrome/Firefox/Safari browsers
   - iOS/Android mobile devices
   - mDNS on macOS/Windows/Linux

---

## Rollback Plan

If issues arise:

1. Comment out `server.begin()` in setup
2. Dashboard feature is completely isolated
3. No changes to core sensor/upload logic
4. Can disable without affecting main functionality

---

## Documentation Updates Needed

- Update README.md with dashboard usage instructions
- Add screenshots of web interface
- Document API endpoints
- Add troubleshooting section
- Update wiring diagrams if display added

---

## References

- [ESPAsyncWebServer Library](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson Documentation](https://arduinojson.org/)
- [WebSocket Protocol](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API)
- [EPA AQI Standards](https://www.airnow.gov/aqi/aqi-basics/)

---

**Next Step:** Begin Phase 1 implementation after approval.
