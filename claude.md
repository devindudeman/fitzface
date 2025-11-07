# FitzFace Pebble Watchface - Development Guide

## Project Overview

FitzFace is a high-density, data-rich watchface for the Pebble 2 Duo (Diorite platform) built with Pebble SDK 4.x. It combines weather data from Open-Meteo, air quality information, NOAA tide predictions, and real-time SF MUNI bus tracking into a clean, symmetrical, power-efficient display.

**Target Device**: Pebble 2 Duo (Diorite) - 144x168 monochrome e-paper display

---

## Architecture

### Three-Layer Architecture

```
┌─────────────────────────────────────┐
│   Pebble C Application (Watch)      │  ← UI rendering, data display, persistence
│   src/c/fitzface.c (1000+ lines)    │
└─────────────────────────────────────┘
              ↕ AppMessage
┌─────────────────────────────────────┐
│   JavaScript Companion (Phone)      │  ← API calls, data aggregation
│   src/pkjs/index.js (800+ lines)    │
└─────────────────────────────────────┘
              ↕ HTTP
┌─────────────────────────────────────┐
│   External APIs                     │  ← Open-Meteo, NOAA, Nominatim, 511.org
│   - Weather & Forecast              │
│   - Air Quality (AQI)               │
│   - Tide Predictions                │
│   - MUNI Bus Tracking               │
│   - Reverse Geocoding               │
└─────────────────────────────────────┘
```

### File Structure

```
fitzface/
├── src/
│   ├── c/
│   │   └── fitzface.c          # Main watchface (C)
│   └── pkjs/
│       ├── index.js            # Companion app (JavaScript)
│       └── config.js           # Clay configuration UI
├── resources/                  # Icons and assets (future)
├── build/
│   └── fitzface.pbw           # Compiled watchface bundle
├── package.json               # Project config & message keys
├── wscript                    # Build configuration
├── README.md                  # User documentation
└── claude.md                  # This file
```

---

## Key Technologies

### C Application (`src/c/fitzface.c`)

- **Pebble SDK 4.x**: Core framework
- **TextLayers**: 11 text layers (including dynamic alert layer)
- **BitmapLayers**: Weather icons (today + tomorrow), arrow indicators (↓↑), tide wave
- **Custom Layer**: Drawing divider lines and black header
- **Vibration**: `vibes_short_pulse()` for alert notifications
- **Persistent Storage**: `persist_write_*` / `persist_read_*` for caching
- **AppMessage**: Communication with phone
- **Tick Timer**: Minute-based updates, weather refresh every 30 minutes

### JavaScript Companion (`src/pkjs/index.js`)

- **Geolocation API**: Gets user coordinates
- **XMLHttpRequest**: Fetches from multiple APIs
- **Weather Alert Detection**: Analyzes 24-hour forecast for unusual conditions
- **Alert Prioritization**: Shows highest severity alert with time range
- **Pebble Clay**: Configuration UI framework
- **localStorage**: Settings persistence

### Data Sources

1. **Open-Meteo Weather API**: `https://api.open-meteo.com/v1/forecast`
   - Current & tomorrow's weather with icons
   - Hourly forecast: weather codes, precipitation probability, wind gusts, UV index
   - Temperature, wind, UV index, sunrise/sunset
   - No API key required

2. **Open-Meteo Air Quality**: `https://air-quality-api.open-meteo.com/v1/air-quality`
   - US AQI (Air Quality Index)
   - Used for current conditions and health alerts (≥100 = High AQI)

3. **NOAA Tides**: `https://api.tidesandcurrents.noaa.gov/api/prod/datagetter`
   - High/low tide predictions
   - Requires station ID configuration

4. **511.org MUNI Bus API**: `https://api.511.org/transit/StopMonitoring`
   - Real-time SF MUNI bus arrival predictions
   - Requires free API key from 511.org/open-data/token
   - SIRI format with stop monitoring endpoint
   - Filters by route, stop code, and direction

5. **Google Pollen API**: `https://pollen.googleapis.com/v1/forecast:lookup`
   - Daily pollen index for tree, grass, and weed (0-5 scale)
   - Requires free API key from Google Cloud Console
   - Used for current conditions and health alerts (≥4 = High Pollen)

6. **Nominatim Reverse Geocoding**: `https://nominatim.openstreetmap.org/reverse`
   - Converts GPS coordinates to city name
   - OpenStreetMap data, no API key required

---

## Display Architecture

### Screen Layout (144x168 pixels)

```
┌────────────────────────────┐  y=0
│ 01  San Francisco      P0  │  Header: Rain% (left) | City Name (center) | Pollen (right)
├────────────────────────────┤  y=20 (DIVIDER)
│ ☀    Wed, Nov 06       ☁   │  Today/Date/Tomorrow icons
│        19:09               │  Large Time (centered)
│     High UV 11AM-3PM       │  Weather/Health Alert (y=80, dynamic, only when active)
├────────────────────────────┤  y=96 (DIVIDER - 3x3 GRID STARTS)
│ 3,12 │  58°  │ 40|68       │  Row 1: MUNI buses | Current temp | Lo|Hi
├────────────────────────────┤  y=120 (DIVIDER)
│ 5mph │  UV3  │ AQI28       │  Row 2: Wind | UV | AQI
├────────────────────────────┤  y=144 (DIVIDER)
│  L   │ ↑6:24 │ ↓7:24       │  Row 3: Tide | Sunrise | Sunset
│ 3:59 │       │             │
└────────────────────────────┘  y=168
```

**Header Bar (y=0-20):**
- **Left (x=4)**: Precipitation probability - 2-digit percentage (e.g., "01" = 1%, "89" = 89%)
- **Center**: City name from GPS reverse geocoding
- **Right (x=110)**: Pollen level - type + index (e.g., "T4" = Tree pollen level 4, "P0" = no pollen)

**3x3 Grid Layout (48px × 24px cells):**
- Top-left cell: MUNI bus countdown (e.g., "3,12" for 3 and 12 minutes)
- All cells aligned and centered for balance

### Design Principles

1. **Symmetry**: Left/right balance with centered city name and data
2. **Hierarchy**: Large time as focal point, dividers separate sections
3. **Visual Indicators**: Weather icons (today + tomorrow), arrow indicators (↓low, ↑high, sunrise↑, sunset↓, tide wave)
4. **Contrast**: Bold headers, regular data text, black background header
5. **Spacing**: Consistent vertical rhythm between sections
6. **Alignment**: Centered primary data (city, time, current temp), symmetrical secondary data

---

## Critical Best Practices

### 1. **NEVER Use Unicode Characters in Text**

**Problem**: Pebble fonts don't support unicode → renders as black boxes

**Solution**: Use ASCII-only characters or bitmap icons

```c
// ❌ WRONG - Unicode characters in text
snprintf(buffer, size, "↑%s ↓%s", sunrise, sunset);
snprintf(buffer, size, "%d°F", temp);

// ✅ CORRECT - ASCII only in text
snprintf(buffer, size, "%dF", temp);      // No degree symbol

// ✅ CORRECT - Use bitmap icons for visual indicators
s_arrow_up_layer = bitmap_layer_create(GRect(x, y, 10, 10));
s_arrow_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ARROW_UP);
```

**Lesson Learned**: Use bitmap icons (PNG resources) for arrows and symbols, ASCII-only for text!

### 2. **Always Clear Emulator Cache After Code Changes**

**Problem**: Pebble emulator caches `.pbw` files and persistent storage, showing old versions even after rebuild

**Solution**: Use `pebble wipe` before testing

```bash
# Complete clean install workflow
pebble kill              # Stop emulator
pebble wipe             # Clear cache (CRITICAL!)
pebble clean            # Clean build artifacts
pebble build            # Rebuild
pebble install --emulator diorite
```

**Cache Locations** (macOS):
- Flash storage: `/Users/[user]/Library/Application Support/Pebble SDK/4.9.75/diorite/qemu_spi_flash.bin`
- App cache: `/Users/[user]/Library/Application Support/Pebble SDK/4.9.75/diorite/app_cache/[UUID].pbw`
- JS storage: `/Users/[user]/Library/Application Support/Pebble SDK/4.9.75/diorite/localstorage/[UUID].dat`

### 3. **Power Optimization Strategies**

```c
// ✅ GOOD: Minute ticks for time updates
tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

// ✅ GOOD: Hourly weather updates only
if (tick_time->tm_min == 0) {
  request_weather_update();
}

// ❌ BAD: Second ticks drain battery
tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

// ❌ BAD: Frequent API calls
tick_timer_service_subscribe(MINUTE_UNIT, request_weather_update);
```

### 4. **Memory Management**

**Current Usage**:
- **Diorite**: 7.4KB / 64KB (88% free) ✅
- **Aplite**: 7.4KB / 24KB (69% free) ✅
- **Resources**: 5.3KB (Diorite), 4.6KB (Aplite)

**Guidelines**:
- Reuse text layers, don't recreate
- Use `static` buffers for strings
- Destroy all layers and bitmaps in `window_unload`
- Keep bitmap resources small (20x20px max for icons)

### 5. **AppMessage Buffer Sizes**

```c
// Large enough for all data fields
const int inbox_size = 512;   // Receiving weather data
const int outbox_size = 128;  // Sending requests
app_message_open(inbox_size, outbox_size);
```

**Message Keys** are auto-generated from `package.json` - never hardcode IDs!

### 6. **Persistent Storage Best Practices**

```c
// ✅ GOOD: Check existence before reading
if (persist_exists(PERSIST_KEY_TEMPERATURE)) {
  temperature = persist_read_int(PERSIST_KEY_TEMPERATURE);
} else {
  temperature = 0;  // Default value
}

// ✅ GOOD: Save after receiving new data
save_weather_data();  // Survive app reloads

// ❌ BAD: Read without checking existence
temperature = persist_read_int(PERSIST_KEY_TEMPERATURE);  // May crash!
```

### 7. **Weather & Health Alert System**

**Alert Detection** (`src/pkjs/index.js:510-634`):
- Analyzes next 24 hours of hourly forecast data
- Checks for: precipitation ≥30%, wind gusts ≥20mph, severe weather codes, UV index, AQI, pollen levels
- Prioritizes alerts: Thunderstorms > Freezing > Snow > Heavy Rain > Fog > **UV/AQI/Pollen** > Wind > Rain

**Weather Alerts:**
- Precipitation, wind, fog, snow, thunderstorms, freezing conditions

**Health Alerts:**
- **High UV**: UV index ≥8 (very high - wear sunscreen!)
- **Extreme UV**: UV index ≥11 (extreme - essential protection)
- **High AQI**: AQI ≥100 (unhealthy for sensitive groups)
- **Unhealthy AQI**: AQI ≥150 (unhealthy for everyone - consider mask)
- **High Pollen**: Pollen level ≥4 (high - allergy precautions recommended)
- **Very High Pollen**: Pollen level 5 (very high - stay indoors if sensitive)

**Alert Display** (`src/c/fitzface.c:809-815`):
- Dynamic text layer at y=80, only shown when active
- Cleared on app init to prevent stale alerts
- Format: "Condition StartTime-EndTime" (e.g., "High UV 11AM-3PM", "High Tree Pollen", "Fog 11AM-10AM")

**Vibration** (`src/c/fitzface.c:589-591`):
- `vibes_short_pulse()` triggered once per new alert
- Compares incoming alert state with previous state
- Only vibrates when alert transitions from inactive to active

**Key Thresholds**:
```javascript
// Precipitation probability
if (precip >= 30) { /* Alert */ }

// Wind gusts
if (windGust >= 20) { /* Alert */ }  // in mph

// UV Index (hourly forecast)
if (uvIndex >= 8) { /* High UV alert */ }
if (uvIndex >= 11) { /* Extreme UV alert */ }

// AQI (current conditions)
if (aqi >= 100) { /* High AQI alert */ }
if (aqi >= 150) { /* Unhealthy AQI alert */ }

// Pollen (current conditions)
if (pollen >= 4) { /* High Pollen alert */ }
if (pollen === 5) { /* Very High Pollen alert */ }

// Weather codes
if (code === 45 || code === 48) { /* Fog */ }
if (code >= 95 && code <= 99) { /* Thunderstorm */ }
if (code >= 66 && code <= 67) { /* Freezing rain */ }
```

**Update Frequency**:
- Weather data fetched every 30 minutes (:00 and :30)
- Alert detection runs on each weather update
- UV alerts check hourly forecast for the day ahead
- AQI and pollen alerts check current conditions only
- Stale alerts cleared on watchface init

### 8. **MUNI Bus Tracking System**

**Configuration** (`src/pkjs/config.js:118-179`):
- Enable/disable toggle for bus countdown
- 511.org API key input (free registration at 511.org/open-data/token)
- Stop code input (5-digit code from 511.org)
- Route selection (e.g., "38R", "1", "N")
- Direction selection (Inbound/Outbound)

**API Integration** (`src/pkjs/index.js:340-468`):
- Fetches from 511.org SIRI StopMonitoring API
- Authenticates with API key in URL parameter
- Filters predictions by route and direction
- Handles multiple time field names (AimedArrivalTime, ExpectedArrivalTime, etc.)
- Sorts arrivals chronologically
- **Timestamp-Based Architecture**: Stores Unix timestamps instead of relative minutes for accurate countdown

**Extrapolation Logic** (`src/pkjs/index.js:449-468`):
- **Problem Solved**: 511.org API typically returns only 3 predictions (~15 minutes), but watchface syncs every 30 minutes
- **Solution**: Mathematical extrapolation to ensure continuous coverage
- **Algorithm**:
  1. Receives 3 real bus arrival timestamps from API (e.g., 3min, 9min, 15min)
  2. Calculates average interval: `totalInterval / (count - 1)` (e.g., 6 minutes)
  3. Extrapolates additional timestamps: `lastTimestamp + avgInterval` (e.g., 21min, 27min, 33min)
  4. Returns 6 total timestamps covering ~36 minutes
- **Result**: Full 30-minute sync window coverage with no data gaps

**Data Processing**:
- Returns 6 Unix timestamps (seconds since epoch)
- JavaScript sends timestamps via `MUNI_TIMESTAMP_1` through `MUNI_TIMESTAMP_6` message keys
- Returns 0 for missing data (displays smiley face placeholder)

**Watch-Side Display** (`src/c/fitzface.c:300-329`):
- Stores 6 timestamps in `time_t muni_timestamps[6]` array
- **Every-Minute Recalculation** in `update_muni_display()`:
  1. Gets current time: `time(NULL)`
  2. Loops through all 6 timestamps
  3. Calculates minutes remaining: `(timestamp - now) / 60`
  4. Filters out past arrivals (only shows future buses)
  5. Displays next 2 future buses
- **Display Formats**:
  - Two buses: "3, 12" (3 and 12 minutes)
  - One bus: "5" (5 minutes)
  - No buses: ":)" (placeholder)
- Top-left grid cell (0-48px, 96-116px)

**Persistence** (`src/c/fitzface.c:527-537, 587-597`):
- Saves all 6 timestamps to persistent storage
- Survives watchface reloads and phone disconnections
- Uses non-sequential persistence keys due to package.json ordering

**Configuration Input Handling** (`src/pkjs/index.js:783-789`):
- Trims whitespace from API key, stop code, and route inputs
- Critical: Whitespace in API key causes 401 authentication failures
- Stores configuration in localStorage for persistence

**Update Frequency**:
- **API Sync**: Every 30 minutes (at :00 and :30) with weather data
- **Display Update**: Every minute via tick handler
- Updates in parallel with weather, AQI, and tide requests

**Technical Advantages**:
- **Accurate**: Countdown always shows correct time, not frozen values
- **Efficient**: No additional API calls needed beyond 30-minute sync
- **Robust**: Extrapolation handles API limitations transparently
- **Persistent**: Timestamps survive app reloads

**Common Issues**:
- **401 Unauthorized**: Check API key for leading/trailing whitespace
- **No predictions**: Verify stop code, route, and direction are correct
- **Empty response**: Bus may not be running (late night, holidays, route changes)
- **Stale JavaScript**: Phone caches JS separately from .pbw - do full uninstall/reinstall if changes don't appear

### 9. **Precipitation Probability Display**

**Data Source**: Open-Meteo hourly forecast
- Automatically fetched with weather data every 30 minutes
- Uses current hour's precipitation probability (0-100%)
- No additional API calls required

**Display** (`src/c/fitzface.c:376-383`):
- Top-left corner of header (position: 4, 3)
- Shows 2-digit percentage with leading zero (e.g., "01" = 1%, "89" = 89%, "100" = 100%)
- Format: `%02d` ensures consistent width
- Uses bold font (GOTHIC_14_BOLD) with inverted colors for header visibility

**Data Processing** (`src/pkjs/index.js:849-852`):
- Extracts `precipitation_probability[0]` from hourly forecast array
- Sends as integer (0-100) via `PRECIPITATION_PROBABILITY` message key
- Falls back to 0 if data unavailable

**Persistence** (`src/c/fitzface.c:71, 560-561, 618`):
- Stored in `PERSIST_KEY_PRECIPITATION_PROBABILITY` (key 25)
- Survives watchface reloads
- Updates every 30 minutes with weather sync

**Color Theme Support** (`src/c/fitzface.c:846`):
- Automatically updates text color when switching themes
- White text on black header (normal mode)
- Black text on white header (inverted mode)

**Update Frequency**:
- Syncs every 30 minutes with weather, AQI, tide, MUNI, and pollen requests
- Always shows current hour's probability for immediate relevance

---

### 10. **Pollen Tracking System**

**Configuration** (`src/pkjs/config.js:180-210`):
- Enable/disable toggle for pollen display
- Google Pollen API key input (free registration at Google Cloud Console)
- Enables pollen.googleapis.com API in Google Cloud project

**API Integration** (`src/pkjs/index.js:467-553`):
- Fetches from Google Pollen API with GPS coordinates
- Returns daily pollen indices for tree, grass, and weed (0-5 scale)
- Parses `dailyInfo[0].pollenTypeInfo` array
- Extracts index values for each pollen type

**Data Processing** (`src/pkjs/index.js:513-553`):
- Identifies pollen type codes: TREE, GRASS, WEED
- Returns indices 0-5 where:
  - 0-1: None/Low
  - 2-3: Moderate
  - 4: High
  - 5: Very High

**Display** (`src/c/fitzface.c:385-409`):
- Top-right corner of header (position: 110, 3)
- Shows highest pollen type with level (e.g., "T4", "G3", "W2")
- Type indicators: T=Tree, G=Grass, W=Weed, P=Pollen (when all types are 0)
- Always displays pollen field when pollen data is available (shows "P0" when all are 0)
- Uses bold font (GOTHIC_14_BOLD) with inverted colors for header visibility

**Alert Integration** (`src/pkjs/index.js:727-751`):
- Triggers alert when any pollen type ≥4
- Priority 5.5 (same as UV/AQI health alerts)
- Shows "High [Type] Pollen" or "Very High [Type] Pollen"
- Identifies which pollen type triggered alert (tree, grass, or weed)

**Update Frequency**:
- Pollen data fetched every 30 minutes with weather data
- Updates in parallel with weather, AQI, tide, and MUNI requests

**Common Issues**:
- **403 Forbidden**: API key doesn't have Pollen API enabled in Google Cloud
- **No data**: API may not have coverage for current location
- **Shows P0**: Normal behavior when all pollen types are 0 (permanent feature)

---

## Common Issues & Solutions

### Issue: Black Boxes Appearing

**Cause**: Unicode characters in strings
**Solution**: Remove unicode or use bitmap icons
- `°` → remove (e.g., "72F" instead of "72°F")
- Arrows/symbols → use bitmap icons via `gbitmap_create_with_resource()`
- For simple indicators, use ASCII: `^` `v` `<` `>`

### Issue: Changes Not Appearing

**Cause**: Emulator cache
**Solution**: `pebble wipe` before install

### Issue: Screenshot Command Hangs

**Cause**: Connection issues to emulator
**Solution**:
```bash
# Kill and restart emulator
pebble kill
pebble install --emulator diorite
# Then take screenshot
pebble screenshot --emulator diorite output.png
```

### Issue: JavaScript Not Loading

**Cause**: Syntax errors or missing dependencies
**Solution**:
- Check build output for webpack errors
- Verify `pebble-clay` is installed: `npm install`
- Check browser console in Pebble app settings

### Issue: AppMessage Failures

**Cause**: Buffer too small or rapid messages
**Solution**:
- Increase buffer sizes (512+ for inbox)
- Add delays between messages
- Check message key spelling (case-sensitive!)

---

## Development Workflow

### Quick Iteration

```bash
# Make code changes
vim src/c/fitzface.c

# Build and test (without cache issues)
pebble build
pebble install --emulator diorite

# If changes don't appear, clear cache
pebble kill && pebble wipe && pebble build && pebble install --emulator diorite
```

### Testing on Physical Watch

```bash
# Build .pbw
pebble build

# Install via phone
# Option 1: Use Pebble app's Developer Connection
pebble install --phone [YOUR_PHONE_IP]

# Option 2: Sideload .pbw file
# Transfer build/fitzface.pbw to phone
# Install via Rebble app
```

### Configuration Testing

1. Install watchface on emulator/watch
2. Open Pebble app on phone
3. Navigate to Settings → FitzFace
4. Changes saved to localStorage and sent to watch
5. Watch receives config via AppMessage inbox

---

## Code Style & Organization

### C Code Structure

```c
// 1. Defines and constants at top
#define KEY_TEMPERATURE MESSAGE_KEY_TEMPERATURE
#define PERSIST_KEY_TEMPERATURE 1

// 2. Static variables (UI elements, data)
static Window *s_main_window;
static TextLayer *s_time_layer;
static WeatherData s_weather_data;

// 3. Forward declarations
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);

// 4. Helper functions
static void format_time_from_timestamp(int timestamp, char *buffer, size_t size) {
  // Implementation
}

// 5. Update functions
static void update_weather_display() {
  // Implementation
}

// 6. Event handlers
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Implementation
}

// 7. Lifecycle (init, load, unload, deinit)
static void main_window_load(Window *window) {
  // Create layers
}

static void main_window_unload(Window *window) {
  // Destroy layers
}

static void init() {
  // Create window, subscribe to services
}

static void deinit() {
  // Cleanup
}

// 8. Main entry point
int main(void) {
  init();
  app_event_loop();
  deinit();
}
```

### JavaScript Code Structure

```javascript
// 1. Requires and dependencies
var Clay = require('pebble-clay');
var clayConfig = require('./config');

// 2. Configuration and state
var CONFIG = { /* defaults */ };
var lastLocation = null;

// 3. Utility functions
function formatNOAADate(date) { }

// 4. API functions
function fetchWeather(location, callback) { }
function fetchAQI(location, callback) { }
function fetchTides(callback) { }

// 5. Data aggregation
function updateWeather() { }
function sendDataToWatch(location, weatherData, aqiData, tideData) { }

// 6. Pebble event handlers
Pebble.addEventListener('ready', function(e) { });
Pebble.addEventListener('appmessage', function(e) { });
Pebble.addEventListener('webviewclosed', function(e) { });
```

---

## Configuration (Clay)

### Structure (`src/pkjs/config.js`)

```javascript
module.exports = [
  {
    "type": "heading",
    "defaultValue": "Section Title"
  },
  {
    "type": "toggle",
    "messageKey": "SHOW_WIND",  // Must match package.json
    "label": "Show Wind Speed",
    "defaultValue": true
  },
  {
    "type": "input",
    "messageKey": "TIDE_STATION",
    "label": "NOAA Station ID",
    "defaultValue": "9414290"
  }
];
```

**Key Points**:
- `messageKey` must match `package.json` messageKeys array
- Use `type: "submit"` at the end
- Configuration auto-sent to watch on save

---

## Performance Metrics

### Build Time
- Clean build: ~1-2 seconds
- Incremental: <1 second

### Battery Impact
- **Time updates**: Every 1 minute
- **Weather updates**: Every 60 minutes
- **Expected battery life**: Several days (similar to other watchfaces)

### Network Usage
- **Weather API**: ~2KB per request
- **AQI API**: ~1KB per request
- **Tides API**: ~5KB per request
- **MUNI API**: ~1-2KB per request
- **Pollen API**: ~1KB per request
- **Total**: ~10-12KB/hour when connected (with all features enabled)

---

## Future Enhancements

### Completed Features
- [x] Weather icons (today + tomorrow) ✅
- [x] Color inversion theme ✅
- [x] Weather alerts with time ranges ✅
- [x] UV/AQI health alerts ✅
- [x] Pollen tracking and alerts ✅
- [x] Forecast view analysis (24 hours ahead) ✅
- [x] Real-time MUNI bus tracking (SF Bay Area) ✅
- [x] City name from GPS ✅

### Potential Features
- [ ] Customizable data field positions
- [ ] Multiple tide stations
- [ ] Step count integration
- [ ] Battery level indicator
- [ ] Support for other transit agencies (BART, Caltrain, etc.)
- [ ] User-configurable alert thresholds
- [ ] Multiple time zones

### Known Limitations
- No color support (Diorite is B&W)
- Limited to ~12 text layers (memory constraint)
- API calls require phone connection
- Tide data requires manual station configuration
- MUNI tracking requires 511.org API key (free but registration required)
- MUNI feature specific to SF Bay Area transit
- Pollen tracking requires Google Pollen API key (free but registration required)
- Pollen API coverage may vary by location

---

## Debugging Tips

### Enable Verbose Logging

```bash
# Watch app logs
pebble logs --emulator diorite

# JavaScript logs (in browser console when configuring)
# Add console.log() statements in src/pkjs/index.js
```

### Common Log Messages

```c
// C side
APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %d", temperature);
APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped: %d", reason);
APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
```

```javascript
// JS side
console.log('Weather data received:', response);
console.log('Sending to watch:', message);
```

### Memory Debugging

```bash
# Check memory usage in build output
pebble build | grep "Free RAM"

# Output shows:
# Free RAM available (heap): 60482 bytes (Diorite)
# Free RAM available (heap): 19522 bytes (Aplite)
```

---

## Resources

### Documentation
- [Pebble Developer Docs](https://developer.rebble.io/developer.pebble.com/docs/index.html)
- [Pebble SDK on GitHub](https://github.com/pebble/pebble-sdk)
- [Clay Configuration](https://github.com/pebble/clay)
- [Open-Meteo API Docs](https://open-meteo.com/en/docs)
- [NOAA Tides API](https://api.tidesandcurrents.noaa.gov/api/prod/)

### Community
- [Rebble Alliance](https://rebble.io/) - Keeps Pebble alive!
- [Pebble Dev Discord](https://discordapp.com/invite/aRUAYFN)
- [r/pebble](https://reddit.com/r/pebble)

### Tools
- **Pebble SDK**: 4.9.75 (latest)
- **Emulator**: Diorite (Pebble 2) + Aplite (original)
- **Languages**: C (watch), JavaScript (phone)

---

## Maintenance Checklist

### Before Each Release
- [ ] `pebble clean && pebble build` - Clean build
- [ ] Test on emulator (Diorite + Aplite)
- [ ] Test on physical watch
- [ ] Verify all API calls work
- [ ] Check configuration UI
- [ ] Test offline mode (cached data)
- [ ] Verify no unicode characters (`grep -r "[^[:print:]]" src/`)
- [ ] Update version in `package.json`
- [ ] Update `README.md` changelog

### Monthly
- [ ] Check API endpoints still working
- [ ] Verify NOAA tide stations active
- [ ] Test with latest Rebble services
- [ ] Review memory usage for leaks

---

## License & Credits

**Built by**: Devin Bernosky
**Platform**: Pebble SDK 4.x / Rebble
**APIs**: Open-Meteo (weather, AQI), NOAA (tides), 511.org (MUNI), Google Pollen API, Nominatim (geocoding)
**License**: GPL-3.0

---

*Last Updated*: 2025-11-06
*SDK Version*: 4.9.75
*Target*: Pebble 2 Duo (Diorite)
