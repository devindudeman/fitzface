#include <pebble.h>

// Message Keys (auto-generated IDs will be used from package.json)
#define KEY_TEMPERATURE MESSAGE_KEY_TEMPERATURE
#define KEY_WIND_SPEED MESSAGE_KEY_WIND_SPEED
#define KEY_UV_INDEX MESSAGE_KEY_UV_INDEX
#define KEY_WEATHER_CODE MESSAGE_KEY_WEATHER_CODE
#define KEY_WEATHER_CODE_TOMORROW MESSAGE_KEY_WEATHER_CODE_TOMORROW
#define KEY_TEMP_MAX MESSAGE_KEY_TEMP_MAX
#define KEY_TEMP_MIN MESSAGE_KEY_TEMP_MIN
#define KEY_SUNRISE MESSAGE_KEY_SUNRISE
#define KEY_SUNSET MESSAGE_KEY_SUNSET
#define KEY_WIND_MAX MESSAGE_KEY_WIND_MAX
#define KEY_AQI MESSAGE_KEY_AQI
#define KEY_PRECIPITATION_PROBABILITY MESSAGE_KEY_PRECIPITATION_PROBABILITY
#define KEY_TIDE_NEXT_TIME MESSAGE_KEY_TIDE_NEXT_TIME
#define KEY_TIDE_NEXT_TYPE MESSAGE_KEY_TIDE_NEXT_TYPE
#define KEY_TIDE_NEXT_HEIGHT MESSAGE_KEY_TIDE_NEXT_HEIGHT
#define KEY_LOCATION_NAME MESSAGE_KEY_LOCATION_NAME
#define KEY_LAST_UPDATE MESSAGE_KEY_LAST_UPDATE
#define KEY_ALERT_TEXT MESSAGE_KEY_ALERT_TEXT
#define KEY_ALERT_ACTIVE MESSAGE_KEY_ALERT_ACTIVE

// Config Keys
#define KEY_CONFIG_TEMP_UNIT MESSAGE_KEY_CONFIG_TEMP_UNIT
#define KEY_CONFIG_WIND_UNIT MESSAGE_KEY_CONFIG_WIND_UNIT
#define KEY_CONFIG_SHOW_AQI MESSAGE_KEY_CONFIG_SHOW_AQI
#define KEY_CONFIG_SHOW_UV MESSAGE_KEY_CONFIG_SHOW_UV
#define KEY_CONFIG_SHOW_WIND MESSAGE_KEY_CONFIG_SHOW_WIND
#define KEY_CONFIG_SHOW_TIDE MESSAGE_KEY_CONFIG_SHOW_TIDE
#define KEY_CONFIG_SHOW_SUNRISE MESSAGE_KEY_CONFIG_SHOW_SUNRISE
#define KEY_CONFIG_INVERT MESSAGE_KEY_CONFIG_INVERT

// MUNI Keys (timestamps)
#define KEY_MUNI_TIMESTAMP_1 MESSAGE_KEY_MUNI_TIMESTAMP_1
#define KEY_MUNI_TIMESTAMP_2 MESSAGE_KEY_MUNI_TIMESTAMP_2
#define KEY_MUNI_TIMESTAMP_3 MESSAGE_KEY_MUNI_TIMESTAMP_3
#define KEY_MUNI_TIMESTAMP_4 MESSAGE_KEY_MUNI_TIMESTAMP_4
#define KEY_MUNI_TIMESTAMP_5 MESSAGE_KEY_MUNI_TIMESTAMP_5
#define KEY_MUNI_TIMESTAMP_6 MESSAGE_KEY_MUNI_TIMESTAMP_6

// Pollen Keys
#define KEY_POLLEN_TREE MESSAGE_KEY_POLLEN_TREE
#define KEY_POLLEN_GRASS MESSAGE_KEY_POLLEN_GRASS
#define KEY_POLLEN_WEED MESSAGE_KEY_POLLEN_WEED

// Persistence Keys
#define PERSIST_KEY_TEMPERATURE 1
#define PERSIST_KEY_WIND_SPEED 2
#define PERSIST_KEY_UV_INDEX 3
#define PERSIST_KEY_WEATHER_CODE 4
#define PERSIST_KEY_WEATHER_CODE_TOMORROW 13
#define PERSIST_KEY_AQI 5
#define PERSIST_KEY_TEMP_MAX 6
#define PERSIST_KEY_TEMP_MIN 7
#define PERSIST_KEY_TIDE_TIME 8
#define PERSIST_KEY_TIDE_TYPE 9
#define PERSIST_KEY_SUNRISE 10
#define PERSIST_KEY_SUNSET 11
#define PERSIST_KEY_LOCATION 12
#define PERSIST_KEY_ALERT_TEXT 14
#define PERSIST_KEY_ALERT_ACTIVE 15
#define PERSIST_KEY_MUNI_TIMESTAMP_1 16
#define PERSIST_KEY_MUNI_TIMESTAMP_2 17
#define PERSIST_KEY_MUNI_TIMESTAMP_3 21
#define PERSIST_KEY_MUNI_TIMESTAMP_4 22
#define PERSIST_KEY_MUNI_TIMESTAMP_5 23
#define PERSIST_KEY_MUNI_TIMESTAMP_6 24
#define PERSIST_KEY_POLLEN_TREE 18
#define PERSIST_KEY_POLLEN_GRASS 19
#define PERSIST_KEY_POLLEN_WEED 20
#define PERSIST_KEY_PRECIPITATION_PROBABILITY 25

// Configuration persistence
#define PERSIST_KEY_CONFIG_TEMP_UNIT 50
#define PERSIST_KEY_CONFIG_SHOW_AQI 51
#define PERSIST_KEY_CONFIG_SHOW_UV 52
#define PERSIST_KEY_CONFIG_SHOW_WIND 53
#define PERSIST_KEY_CONFIG_SHOW_TIDE 54
#define PERSIST_KEY_CONFIG_SHOW_SUNRISE 55
#define PERSIST_KEY_CONFIG_INVERT 56

// UI Elements
static Window *s_main_window;
static Layer *s_divider_layer;

// Time display
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

// Alert section
static TextLayer *s_alert_layer;
static bool s_alert_active;

// Header section
static TextLayer *s_location_layer;
static BitmapLayer *s_weather_icon_layer;
static GBitmap *s_weather_icon;
static BitmapLayer *s_weather_icon_tomorrow_layer;
static GBitmap *s_weather_icon_tomorrow;

// Data fields
static TextLayer *s_wind_layer;
static TextLayer *s_uv_layer;
static TextLayer *s_aqi_layer;
static TextLayer *s_temp_min_layer;
static TextLayer *s_temp_current_layer;
static TextLayer *s_temp_max_layer;
static BitmapLayer *s_temp_arrow_low_layer;
static BitmapLayer *s_temp_arrow_high_layer;
static GBitmap *s_temp_arrow_low_bitmap;
static GBitmap *s_temp_arrow_high_bitmap;

// MUNI bus tracking (top-left grid cell)
static TextLayer *s_muni_layer;

// Pollen display (top right of header)
static TextLayer *s_pollen_layer;

// Precipitation probability (top left of header)
static TextLayer *s_precip_layer;

// Footer
static TextLayer *s_tide_layer;
static TextLayer *s_sunrise_layer;
static TextLayer *s_sunset_layer;
static BitmapLayer *s_arrow_up_layer;
static BitmapLayer *s_arrow_down_layer;
static BitmapLayer *s_wave_layer;
static GBitmap *s_arrow_up_bitmap;
static GBitmap *s_arrow_down_bitmap;
static GBitmap *s_wave_bitmap;

// Data storage
typedef struct {
  int temperature;
  int wind_speed;
  int uv_index;
  int weather_code;
  int weather_code_tomorrow;
  int temp_max;
  int temp_min;
  int aqi;
  int precipitation_probability;  // 0-100%
  int tide_time;
  int tide_type; // 0 = low, 1 = high
  int sunrise;
  int sunset;
  char location[32];
  char alert_text[64];
  bool alert_active;
  time_t muni_timestamps[6];  // Unix timestamps for next 6 buses (0 = no data)
  int pollen_tree;   // Tree pollen 0-5 (-1 = no data)
  int pollen_grass;  // Grass pollen 0-5 (-1 = no data)
  int pollen_weed;   // Weed pollen 0-5 (-1 = no data)
  time_t last_update;
} WeatherData;

typedef struct {
  bool temp_celsius;
  bool show_aqi;
  bool show_uv;
  bool show_wind;
  bool show_tide;
  bool show_sunrise;
  bool invert_colors;
} Config;

static WeatherData s_weather_data;
static Config s_config;

// Forward declarations
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void inbox_dropped_callback(AppMessageResult reason, void *context);
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
static void outbox_sent_callback(DictionaryIterator *iterator, void *context);
static void load_persisted_data();
static void save_weather_data();
static void load_config();
static void save_config();
static void request_weather_update();
static void update_weather_display();
static void update_muni_display();
static void apply_color_theme();

// Color helpers for inversion support
static GColor get_foreground_color() {
  return s_config.invert_colors ? GColorWhite : GColorBlack;
}

static GColor get_background_color() {
  return s_config.invert_colors ? GColorBlack : GColorWhite;
}

static GCompOp get_bitmap_compositing_mode() {
  // On inverted mode, use OR to make black pixels show as white on black background
  // This is a workaround - proper solution would require inverted bitmap resources
  return s_config.invert_colors ? GCompOpOr : GCompOpSet;
}

// Utility: Format time from Unix timestamp (24-hour format)
static void format_time_from_timestamp(int timestamp, char *buffer, size_t size) {
  time_t t = (time_t)timestamp;
  struct tm *tm_info = localtime(&t);
  strftime(buffer, size, "%H:%M", tm_info);
}

// Get weather icon resource based on weather code
// Open-Meteo weather codes: https://open-meteo.com/en/docs
static uint32_t get_weather_icon_resource(int weather_code, bool use_current_time_for_night) {
  bool is_night = false;
  if (use_current_time_for_night) {
    time_t now = time(NULL);
    is_night = (s_weather_data.sunrise > 0 && s_weather_data.sunset > 0) &&
               (now < s_weather_data.sunrise || now > s_weather_data.sunset);
  }

  bool inverted = s_config.invert_colors;

  switch (weather_code) {
    case 0: // Clear sky
      if (is_night) {
        return inverted ? RESOURCE_ID_ICON_MOON_INVERTED : RESOURCE_ID_ICON_MOON;
      } else {
        return inverted ? RESOURCE_ID_ICON_SUN_INVERTED : RESOURCE_ID_ICON_SUN;
      }

    case 1: // Mainly clear
    case 2: // Partly cloudy
      return inverted ? RESOURCE_ID_ICON_CLOUD_INVERTED : RESOURCE_ID_ICON_CLOUD;

    case 3: // Overcast
    case 45: // Fog
    case 48: // Depositing rime fog
      return inverted ? RESOURCE_ID_ICON_CLOUDS_INVERTED : RESOURCE_ID_ICON_CLOUDS;

    case 51: // Drizzle: Light
    case 53: // Drizzle: Moderate
    case 55: // Drizzle: Dense
    case 56: // Freezing Drizzle: Light
    case 57: // Freezing Drizzle: Dense
      return inverted ? RESOURCE_ID_ICON_RAIN_LIGHT_INVERTED : RESOURCE_ID_ICON_RAIN_LIGHT;

    case 61: // Rain: Slight
    case 66: // Freezing Rain: Light
    case 80: // Rain showers: Slight
      return inverted ? RESOURCE_ID_ICON_RAIN_MEDIUM_INVERTED : RESOURCE_ID_ICON_RAIN_MEDIUM;

    case 63: // Rain: Moderate
    case 65: // Rain: Heavy
    case 67: // Freezing Rain: Heavy
    case 81: // Rain showers: Moderate
    case 82: // Rain showers: Violent
      return inverted ? RESOURCE_ID_ICON_RAIN_HEAVY_INVERTED : RESOURCE_ID_ICON_RAIN_HEAVY;

    case 71: // Snow fall: Slight
    case 73: // Snow fall: Moderate
    case 75: // Snow fall: Heavy
    case 77: // Snow grains
    case 85: // Snow showers: Slight
    case 86: // Snow showers: Heavy
      return inverted ? RESOURCE_ID_ICON_SNOW_INVERTED : RESOURCE_ID_ICON_SNOW;

    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return inverted ? RESOURCE_ID_ICON_LIGHTNING_INVERTED : RESOURCE_ID_ICON_LIGHTNING;

    default:
      if (is_night) {
        return inverted ? RESOURCE_ID_ICON_MOON_INVERTED : RESOURCE_ID_ICON_MOON;
      } else {
        return inverted ? RESOURCE_ID_ICON_SUN_INVERTED : RESOURCE_ID_ICON_SUN;
      }
  }
}

// Update time display
static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_time_buffer[8];
  static char s_date_buffer[16];

  // Time: HH:MM
  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
    // Remove leading zero for 12-hour format
    if (s_time_buffer[0] == '0') {
      memmove(s_time_buffer, s_time_buffer + 1, strlen(s_time_buffer));
    }
  }

  // Date: DAY, MON DD
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %d", tick_time);

  text_layer_set_text(s_time_layer, s_time_buffer);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

// Update MUNI bus countdown display (recalculated every minute from timestamps)
static void update_muni_display() {
  static char muni_buffer[16];
  time_t now = time(NULL);
  int future_buses[6];
  int count = 0;

  // Find all future arrivals from timestamps
  for (int i = 0; i < 6; i++) {
    if (s_weather_data.muni_timestamps[i] > 0) {
      int minutes = (s_weather_data.muni_timestamps[i] - now) / 60;
      if (minutes > 0) {  // Only future arrivals
        future_buses[count++] = minutes;
      }
    }
  }

  // Display next 2 future buses
  if (count >= 2) {
    snprintf(muni_buffer, sizeof(muni_buffer), "%d, %d",
             future_buses[0], future_buses[1]);
  } else if (count == 1) {
    snprintf(muni_buffer, sizeof(muni_buffer), "%d", future_buses[0]);
  } else {
    // No future buses - show placeholder
    strcpy(muni_buffer, ":)");
  }

  text_layer_set_text(s_muni_layer, muni_buffer);
}

// Update weather display
static void update_weather_display() {
  static char wind_buffer[32];
  static char uv_buffer[16];
  static char aqi_buffer[16];
  static char temp_min_buffer[8];
  static char temp_current_buffer[8];
  static char temp_max_buffer[8];

  // Weather icon (uses current time for day/night)
  if (s_weather_icon) {
    gbitmap_destroy(s_weather_icon);
  }
  uint32_t icon_resource = get_weather_icon_resource(s_weather_data.weather_code, true);
  s_weather_icon = gbitmap_create_with_resource(icon_resource);
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon);

  // Tomorrow's weather icon (always show daytime icon)
  if (s_weather_icon_tomorrow) {
    gbitmap_destroy(s_weather_icon_tomorrow);
  }
  uint32_t icon_resource_tomorrow = get_weather_icon_resource(s_weather_data.weather_code_tomorrow, false);
  s_weather_icon_tomorrow = gbitmap_create_with_resource(icon_resource_tomorrow);
  bitmap_layer_set_bitmap(s_weather_icon_tomorrow_layer, s_weather_icon_tomorrow);

  // Location
  text_layer_set_text(s_location_layer, s_weather_data.location);

  // Weather alert (show/hide based on active state)
  if (s_weather_data.alert_active) {
    text_layer_set_text(s_alert_layer, s_weather_data.alert_text);
    layer_set_hidden(text_layer_get_layer(s_alert_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_alert_layer), true);
  }

  // MUNI bus countdown updated separately (recalculated every minute)
  update_muni_display();

  // Precipitation probability display (top left corner) - show as 2-digit percentage
  static char precip_buffer[8];
  if (s_weather_data.precipitation_probability >= 0) {
    snprintf(precip_buffer, sizeof(precip_buffer), "%02d", s_weather_data.precipitation_probability);
    text_layer_set_text(s_precip_layer, precip_buffer);
  } else {
    text_layer_set_text(s_precip_layer, "");
  }

  // Pollen display (top right corner) - show worst type + level
  static char pollen_buffer[8];
  if (s_weather_data.pollen_tree >= 0 || s_weather_data.pollen_grass >= 0 || s_weather_data.pollen_weed >= 0) {
    // Find highest pollen count
    int max_pollen = 0;
    char pollen_type = 'P';  // Default to 'P' for testing when all are 0

    if (s_weather_data.pollen_tree > max_pollen) {
      max_pollen = s_weather_data.pollen_tree;
      pollen_type = 'T';
    }
    if (s_weather_data.pollen_grass > max_pollen) {
      max_pollen = s_weather_data.pollen_grass;
      pollen_type = 'G';
    }
    if (s_weather_data.pollen_weed > max_pollen) {
      max_pollen = s_weather_data.pollen_weed;
      pollen_type = 'W';
    }

    // TEMPORARY: Always show pollen for testing (even when 0)
    snprintf(pollen_buffer, sizeof(pollen_buffer), "%c%d", pollen_type, max_pollen);
    text_layer_set_text(s_pollen_layer, pollen_buffer);
  } else {
    // No pollen data
    text_layer_set_text(s_pollen_layer, "");
  }

  // Temperature grid - centered layout
  // Current temp - center column (large)
  snprintf(temp_current_buffer, sizeof(temp_current_buffer), "%d°", s_weather_data.temperature);
  text_layer_set_text(s_temp_current_layer, temp_current_buffer);

  // Combined low/high - right column (format: "53°|59°" centered)
  snprintf(temp_max_buffer, sizeof(temp_max_buffer), "%d|%d",
           s_weather_data.temp_min, s_weather_data.temp_max);
  text_layer_set_text(s_temp_max_layer, temp_max_buffer);

  // temp_min_layer is hidden, not used
  text_layer_set_text(s_temp_min_layer, "");

  // Wind (if enabled) - with mph unit
  if (s_config.show_wind) {
    snprintf(wind_buffer, sizeof(wind_buffer), "%dmph", s_weather_data.wind_speed);
    text_layer_set_text(s_wind_layer, wind_buffer);
    layer_set_hidden(text_layer_get_layer(s_wind_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_wind_layer), true);
  }

  // UV Index (if enabled) - with UV label
  if (s_config.show_uv) {
    snprintf(uv_buffer, sizeof(uv_buffer), "UV%d", s_weather_data.uv_index);
    text_layer_set_text(s_uv_layer, uv_buffer);
    layer_set_hidden(text_layer_get_layer(s_uv_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_uv_layer), true);
  }

  // AQI (if enabled) - with AQI label
  if (s_config.show_aqi) {
    snprintf(aqi_buffer, sizeof(aqi_buffer), "AQI%d", s_weather_data.aqi);
    text_layer_set_text(s_aqi_layer, aqi_buffer);
    layer_set_hidden(text_layer_get_layer(s_aqi_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_aqi_layer), true);
  }

  // Tide (if enabled) - H/L with 24-hour time
  static char tide_display[16];
  if (s_config.show_tide && s_weather_data.tide_time > 0) {
    char tide_time_str[16];
    format_time_from_timestamp(s_weather_data.tide_time, tide_time_str, sizeof(tide_time_str));
    snprintf(tide_display, sizeof(tide_display), "%s %s",
             s_weather_data.tide_type == 1 ? "H" : "L", tide_time_str);
    text_layer_set_text(s_tide_layer, tide_display);
    layer_set_hidden(text_layer_get_layer(s_tide_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_tide_layer), true);
  }

  // Sunrise/Sunset (if enabled) - bitmap arrows with 24-hour time
  static char sunrise_str[16], sunset_str[16];
  if (s_config.show_sunrise && s_weather_data.sunrise > 0) {
    format_time_from_timestamp(s_weather_data.sunrise, sunrise_str, sizeof(sunrise_str));
    format_time_from_timestamp(s_weather_data.sunset, sunset_str, sizeof(sunset_str));

    // Sunrise - just the time (arrow is a bitmap)
    text_layer_set_text(s_sunrise_layer, sunrise_str);
    layer_set_hidden(text_layer_get_layer(s_sunrise_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(s_arrow_up_layer), false);

    // Sunset - just the time (arrow is a bitmap)
    text_layer_set_text(s_sunset_layer, sunset_str);
    layer_set_hidden(text_layer_get_layer(s_sunset_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(s_arrow_down_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_sunrise_layer), true);
    layer_set_hidden(text_layer_get_layer(s_sunset_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_arrow_up_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_arrow_down_layer), true);
  }
}

// Tick handler - called every minute
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_muni_display();  // Recalculate MUNI countdown every minute

  // Request weather update every 30 minutes (at :00 and :30)
  if (tick_time->tm_min == 0 || tick_time->tm_min == 30) {
    request_weather_update();
  }
}

// Request weather update from phone
static void request_weather_update() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_uint8(iter, 0, 0); // Dummy message to trigger JS
  app_message_outbox_send();
}

// Load persisted weather data
static void load_persisted_data() {
  if (persist_exists(PERSIST_KEY_TEMPERATURE)) {
    s_weather_data.temperature = persist_read_int(PERSIST_KEY_TEMPERATURE);
    s_weather_data.wind_speed = persist_read_int(PERSIST_KEY_WIND_SPEED);
    s_weather_data.uv_index = persist_read_int(PERSIST_KEY_UV_INDEX);
    s_weather_data.weather_code = persist_read_int(PERSIST_KEY_WEATHER_CODE);
    s_weather_data.weather_code_tomorrow = persist_read_int(PERSIST_KEY_WEATHER_CODE_TOMORROW);
    s_weather_data.aqi = persist_read_int(PERSIST_KEY_AQI);
    s_weather_data.temp_max = persist_read_int(PERSIST_KEY_TEMP_MAX);
    s_weather_data.temp_min = persist_read_int(PERSIST_KEY_TEMP_MIN);
    s_weather_data.tide_time = persist_read_int(PERSIST_KEY_TIDE_TIME);
    s_weather_data.tide_type = persist_read_int(PERSIST_KEY_TIDE_TYPE);
    s_weather_data.sunrise = persist_read_int(PERSIST_KEY_SUNRISE);
    s_weather_data.sunset = persist_read_int(PERSIST_KEY_SUNSET);

    if (persist_exists(PERSIST_KEY_LOCATION)) {
      persist_read_string(PERSIST_KEY_LOCATION, s_weather_data.location, sizeof(s_weather_data.location));
    }

    if (persist_exists(PERSIST_KEY_ALERT_TEXT)) {
      persist_read_string(PERSIST_KEY_ALERT_TEXT, s_weather_data.alert_text, sizeof(s_weather_data.alert_text));
    }

    if (persist_exists(PERSIST_KEY_ALERT_ACTIVE)) {
      s_weather_data.alert_active = persist_read_bool(PERSIST_KEY_ALERT_ACTIVE);
    }

    // Load MUNI timestamps
    for (int i = 0; i < 6; i++) {
      int persist_key = PERSIST_KEY_MUNI_TIMESTAMP_1 + i;
      // Skip indices 2-5 since they use non-sequential keys
      if (i == 2) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_3;
      else if (i == 3) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_4;
      else if (i == 4) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_5;
      else if (i == 5) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_6;

      s_weather_data.muni_timestamps[i] = persist_exists(persist_key) ?
                                           persist_read_int(persist_key) : 0;
    }

    // Load pollen data
    s_weather_data.pollen_tree = persist_exists(PERSIST_KEY_POLLEN_TREE) ?
                                  persist_read_int(PERSIST_KEY_POLLEN_TREE) : -1;
    s_weather_data.pollen_grass = persist_exists(PERSIST_KEY_POLLEN_GRASS) ?
                                   persist_read_int(PERSIST_KEY_POLLEN_GRASS) : -1;
    s_weather_data.pollen_weed = persist_exists(PERSIST_KEY_POLLEN_WEED) ?
                                  persist_read_int(PERSIST_KEY_POLLEN_WEED) : -1;
    s_weather_data.precipitation_probability = persist_exists(PERSIST_KEY_PRECIPITATION_PROBABILITY) ?
                                                persist_read_int(PERSIST_KEY_PRECIPITATION_PROBABILITY) : 0;
  } else {
    // Default values
    s_weather_data.temperature = 0;
    s_weather_data.wind_speed = 0;
    s_weather_data.uv_index = 0;
    s_weather_data.weather_code = 0;
    s_weather_data.aqi = 0;
    s_weather_data.temp_max = 0;
    s_weather_data.temp_min = 0;
    s_weather_data.tide_time = 0;
    s_weather_data.tide_type = 0;
    s_weather_data.sunrise = 0;
    s_weather_data.sunset = 0;
    for (int i = 0; i < 6; i++) {
      s_weather_data.muni_timestamps[i] = 0;  // No MUNI data
    }
    s_weather_data.pollen_tree = -1;   // No pollen data
    s_weather_data.pollen_grass = -1;
    s_weather_data.pollen_weed = -1;
    snprintf(s_weather_data.location, sizeof(s_weather_data.location), "Loading...");
  }
}

// Save weather data to persistent storage
static void save_weather_data() {
  persist_write_int(PERSIST_KEY_TEMPERATURE, s_weather_data.temperature);
  persist_write_int(PERSIST_KEY_WIND_SPEED, s_weather_data.wind_speed);
  persist_write_int(PERSIST_KEY_UV_INDEX, s_weather_data.uv_index);
  persist_write_int(PERSIST_KEY_WEATHER_CODE, s_weather_data.weather_code);
  persist_write_int(PERSIST_KEY_WEATHER_CODE_TOMORROW, s_weather_data.weather_code_tomorrow);
  persist_write_int(PERSIST_KEY_AQI, s_weather_data.aqi);
  persist_write_int(PERSIST_KEY_TEMP_MAX, s_weather_data.temp_max);
  persist_write_int(PERSIST_KEY_TEMP_MIN, s_weather_data.temp_min);
  persist_write_int(PERSIST_KEY_TIDE_TIME, s_weather_data.tide_time);
  persist_write_int(PERSIST_KEY_TIDE_TYPE, s_weather_data.tide_type);
  persist_write_int(PERSIST_KEY_SUNRISE, s_weather_data.sunrise);
  persist_write_int(PERSIST_KEY_SUNSET, s_weather_data.sunset);
  persist_write_string(PERSIST_KEY_LOCATION, s_weather_data.location);
  persist_write_string(PERSIST_KEY_ALERT_TEXT, s_weather_data.alert_text);
  persist_write_bool(PERSIST_KEY_ALERT_ACTIVE, s_weather_data.alert_active);

  // Save MUNI timestamps
  for (int i = 0; i < 6; i++) {
    int persist_key = PERSIST_KEY_MUNI_TIMESTAMP_1 + i;
    // Skip indices 2-5 since they use non-sequential keys
    if (i == 2) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_3;
    else if (i == 3) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_4;
    else if (i == 4) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_5;
    else if (i == 5) persist_key = PERSIST_KEY_MUNI_TIMESTAMP_6;

    persist_write_int(persist_key, s_weather_data.muni_timestamps[i]);
  }

  persist_write_int(PERSIST_KEY_POLLEN_TREE, s_weather_data.pollen_tree);
  persist_write_int(PERSIST_KEY_POLLEN_GRASS, s_weather_data.pollen_grass);
  persist_write_int(PERSIST_KEY_POLLEN_WEED, s_weather_data.pollen_weed);
  persist_write_int(PERSIST_KEY_PRECIPITATION_PROBABILITY, s_weather_data.precipitation_probability);
}

// Load configuration
static void load_config() {
  s_config.temp_celsius = persist_exists(PERSIST_KEY_CONFIG_TEMP_UNIT) ?
                          persist_read_bool(PERSIST_KEY_CONFIG_TEMP_UNIT) : false;
  s_config.show_aqi = persist_exists(PERSIST_KEY_CONFIG_SHOW_AQI) ?
                      persist_read_bool(PERSIST_KEY_CONFIG_SHOW_AQI) : true;
  s_config.show_uv = persist_exists(PERSIST_KEY_CONFIG_SHOW_UV) ?
                     persist_read_bool(PERSIST_KEY_CONFIG_SHOW_UV) : true;
  s_config.show_wind = persist_exists(PERSIST_KEY_CONFIG_SHOW_WIND) ?
                       persist_read_bool(PERSIST_KEY_CONFIG_SHOW_WIND) : true;
  s_config.show_tide = persist_exists(PERSIST_KEY_CONFIG_SHOW_TIDE) ?
                       persist_read_bool(PERSIST_KEY_CONFIG_SHOW_TIDE) : true;
  s_config.show_sunrise = persist_exists(PERSIST_KEY_CONFIG_SHOW_SUNRISE) ?
                          persist_read_bool(PERSIST_KEY_CONFIG_SHOW_SUNRISE) : true;
  s_config.invert_colors = persist_exists(PERSIST_KEY_CONFIG_INVERT) ?
                           persist_read_bool(PERSIST_KEY_CONFIG_INVERT) : false;
}

// Save configuration
static void save_config() {
  persist_write_bool(PERSIST_KEY_CONFIG_TEMP_UNIT, s_config.temp_celsius);
  persist_write_bool(PERSIST_KEY_CONFIG_SHOW_AQI, s_config.show_aqi);
  persist_write_bool(PERSIST_KEY_CONFIG_SHOW_UV, s_config.show_uv);
  persist_write_bool(PERSIST_KEY_CONFIG_SHOW_WIND, s_config.show_wind);
  persist_write_bool(PERSIST_KEY_CONFIG_SHOW_TIDE, s_config.show_tide);
  persist_write_bool(PERSIST_KEY_CONFIG_SHOW_SUNRISE, s_config.show_sunrise);
  persist_write_bool(PERSIST_KEY_CONFIG_INVERT, s_config.invert_colors);
}

// AppMessage inbox received callback
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read weather data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *wind_tuple = dict_find(iterator, KEY_WIND_SPEED);
  Tuple *uv_tuple = dict_find(iterator, KEY_UV_INDEX);
  Tuple *weather_code_tuple = dict_find(iterator, KEY_WEATHER_CODE);
  Tuple *weather_code_tomorrow_tuple = dict_find(iterator, KEY_WEATHER_CODE_TOMORROW);
  Tuple *aqi_tuple = dict_find(iterator, KEY_AQI);
  Tuple *precip_prob_tuple = dict_find(iterator, KEY_PRECIPITATION_PROBABILITY);
  Tuple *temp_max_tuple = dict_find(iterator, KEY_TEMP_MAX);
  Tuple *temp_min_tuple = dict_find(iterator, KEY_TEMP_MIN);
  Tuple *tide_time_tuple = dict_find(iterator, KEY_TIDE_NEXT_TIME);
  Tuple *tide_type_tuple = dict_find(iterator, KEY_TIDE_NEXT_TYPE);
  Tuple *sunrise_tuple = dict_find(iterator, KEY_SUNRISE);
  Tuple *sunset_tuple = dict_find(iterator, KEY_SUNSET);
  Tuple *location_tuple = dict_find(iterator, KEY_LOCATION_NAME);
  Tuple *alert_text_tuple = dict_find(iterator, KEY_ALERT_TEXT);
  Tuple *alert_active_tuple = dict_find(iterator, KEY_ALERT_ACTIVE);
  Tuple *muni_timestamp_1_tuple = dict_find(iterator, KEY_MUNI_TIMESTAMP_1);
  Tuple *muni_timestamp_2_tuple = dict_find(iterator, KEY_MUNI_TIMESTAMP_2);
  Tuple *muni_timestamp_3_tuple = dict_find(iterator, KEY_MUNI_TIMESTAMP_3);
  Tuple *muni_timestamp_4_tuple = dict_find(iterator, KEY_MUNI_TIMESTAMP_4);
  Tuple *muni_timestamp_5_tuple = dict_find(iterator, KEY_MUNI_TIMESTAMP_5);
  Tuple *muni_timestamp_6_tuple = dict_find(iterator, KEY_MUNI_TIMESTAMP_6);
  Tuple *pollen_tree_tuple = dict_find(iterator, KEY_POLLEN_TREE);
  Tuple *pollen_grass_tuple = dict_find(iterator, KEY_POLLEN_GRASS);
  Tuple *pollen_weed_tuple = dict_find(iterator, KEY_POLLEN_WEED);

  // Read configuration
  Tuple *temp_unit_tuple = dict_find(iterator, KEY_CONFIG_TEMP_UNIT);
  Tuple *show_aqi_tuple = dict_find(iterator, KEY_CONFIG_SHOW_AQI);
  Tuple *show_uv_tuple = dict_find(iterator, KEY_CONFIG_SHOW_UV);
  Tuple *show_wind_tuple = dict_find(iterator, KEY_CONFIG_SHOW_WIND);
  Tuple *show_tide_tuple = dict_find(iterator, KEY_CONFIG_SHOW_TIDE);
  Tuple *show_sunrise_tuple = dict_find(iterator, KEY_CONFIG_SHOW_SUNRISE);
  Tuple *invert_tuple = dict_find(iterator, KEY_CONFIG_INVERT);

  // Update weather data
  if (temp_tuple) s_weather_data.temperature = (int)temp_tuple->value->int32;
  if (wind_tuple) s_weather_data.wind_speed = (int)wind_tuple->value->int32;
  if (uv_tuple) s_weather_data.uv_index = (int)uv_tuple->value->int32;
  if (weather_code_tuple) s_weather_data.weather_code = (int)weather_code_tuple->value->int32;
  if (weather_code_tomorrow_tuple) s_weather_data.weather_code_tomorrow = (int)weather_code_tomorrow_tuple->value->int32;
  if (aqi_tuple) s_weather_data.aqi = (int)aqi_tuple->value->int32;
  if (precip_prob_tuple) s_weather_data.precipitation_probability = (int)precip_prob_tuple->value->int32;
  if (temp_max_tuple) s_weather_data.temp_max = (int)temp_max_tuple->value->int32;
  if (temp_min_tuple) s_weather_data.temp_min = (int)temp_min_tuple->value->int32;
  if (tide_time_tuple) s_weather_data.tide_time = (int)tide_time_tuple->value->int32;
  if (tide_type_tuple) s_weather_data.tide_type = (int)tide_type_tuple->value->int32;
  if (sunrise_tuple) s_weather_data.sunrise = (int)sunrise_tuple->value->int32;
  if (sunset_tuple) s_weather_data.sunset = (int)sunset_tuple->value->int32;
  if (location_tuple) {
    snprintf(s_weather_data.location, sizeof(s_weather_data.location), "%s", location_tuple->value->cstring);
  }

  // Update MUNI timestamps
  Tuple *muni_tuples[6] = {
    muni_timestamp_1_tuple, muni_timestamp_2_tuple, muni_timestamp_3_tuple,
    muni_timestamp_4_tuple, muni_timestamp_5_tuple, muni_timestamp_6_tuple
  };
  for (int i = 0; i < 6; i++) {
    if (muni_tuples[i]) {
      s_weather_data.muni_timestamps[i] = (time_t)muni_tuples[i]->value->int32;
    }
  }

  if (pollen_tree_tuple) s_weather_data.pollen_tree = (int)pollen_tree_tuple->value->int32;
  if (pollen_grass_tuple) s_weather_data.pollen_grass = (int)pollen_grass_tuple->value->int32;
  if (pollen_weed_tuple) s_weather_data.pollen_weed = (int)pollen_weed_tuple->value->int32;

  // Handle alert data
  bool new_alert = false;
  if (alert_text_tuple) {
    snprintf(s_weather_data.alert_text, sizeof(s_weather_data.alert_text), "%s", alert_text_tuple->value->cstring);
  }
  if (alert_active_tuple) {
    bool was_active = s_weather_data.alert_active;
    s_weather_data.alert_active = (bool)alert_active_tuple->value->int32;
    // Trigger vibration if this is a new alert
    if (s_weather_data.alert_active && !was_active) {
      new_alert = true;
    }
  }

  // Update configuration
  if (temp_unit_tuple) s_config.temp_celsius = (bool)temp_unit_tuple->value->int32;
  if (show_aqi_tuple) s_config.show_aqi = (bool)show_aqi_tuple->value->int32;
  if (show_uv_tuple) s_config.show_uv = (bool)show_uv_tuple->value->int32;
  if (show_wind_tuple) s_config.show_wind = (bool)show_wind_tuple->value->int32;
  if (show_tide_tuple) s_config.show_tide = (bool)show_tide_tuple->value->int32;
  if (show_sunrise_tuple) s_config.show_sunrise = (bool)show_sunrise_tuple->value->int32;

  // Handle color inversion setting change
  if (invert_tuple) {
    bool was_inverted = s_config.invert_colors;
    s_config.invert_colors = (bool)invert_tuple->value->int32;
    // Apply color theme if setting changed
    if (was_inverted != s_config.invert_colors) {
      apply_color_theme();
    }
  }

  // Save to persistent storage
  save_weather_data();
  save_config();

  // Update display
  update_weather_display();

  // Vibrate if new alert
  if (new_alert) {
    vibes_short_pulse();
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// Divider layer update proc - draws visual elements for depth
static void divider_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw header rectangle (adds depth) - inverts with theme
  graphics_context_set_fill_color(ctx, get_foreground_color());
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 20), 0, GCornerNone);

  // Draw divider lines and grid - inverts with theme
  graphics_context_set_stroke_color(ctx, get_foreground_color());
  graphics_context_set_stroke_width(ctx, 1);

  // 3x3 Grid lines - compact 24px boxes anchored to bottom
  // Horizontal dividers
  graphics_draw_line(ctx, GPoint(0, 96), GPoint(bounds.size.w, 96));   // Before temp row (96-120)
  graphics_draw_line(ctx, GPoint(0, 120), GPoint(bounds.size.w, 120)); // Before data row (120-144)
  graphics_draw_line(ctx, GPoint(0, 144), GPoint(bounds.size.w, 144)); // Before time row (144-168)

  // Vertical dividers for 3-column grid (48px per column) - from grid start to bottom
  graphics_draw_line(ctx, GPoint(48, 96), GPoint(48, bounds.size.h));  // Left divider
  graphics_draw_line(ctx, GPoint(96, 96), GPoint(96, bounds.size.h));  // Right divider

  // Box around time area - tall enough to clear 42pt font
  graphics_context_set_stroke_color(ctx, get_foreground_color());
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(4, 26, bounds.size.w - 8, 56), 4);
}

// Create text layer helper
static TextLayer* create_text_layer(GRect frame, GTextAlignment alignment, GFont font) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, get_foreground_color());
  text_layer_set_text_alignment(layer, alignment);
  text_layer_set_font(layer, font);
  return layer;
}

// Create text layer with custom colors
static TextLayer* create_text_layer_colored(GRect frame, GTextAlignment alignment, GFont font, GColor text_color, GColor bg_color) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, bg_color);
  text_layer_set_text_color(layer, text_color);
  text_layer_set_text_alignment(layer, alignment);
  text_layer_set_font(layer, font);
  return layer;
}

// Apply color theme - updates all UI elements when inversion changes
static void apply_color_theme() {
  // Update window background
  window_set_background_color(s_main_window, get_background_color());

  // Update all text layers
  text_layer_set_text_color(s_time_layer, get_foreground_color());
  text_layer_set_text_color(s_date_layer, get_foreground_color());
  text_layer_set_text_color(s_alert_layer, get_foreground_color());
  text_layer_set_text_color(s_location_layer, get_background_color()); // Inverted for header
  text_layer_set_text_color(s_muni_layer, get_foreground_color());
  text_layer_set_text_color(s_temp_min_layer, get_foreground_color());
  text_layer_set_text_color(s_temp_current_layer, get_foreground_color());
  text_layer_set_text_color(s_temp_max_layer, get_foreground_color());
  text_layer_set_text_color(s_wind_layer, get_foreground_color());
  text_layer_set_text_color(s_uv_layer, get_foreground_color());
  text_layer_set_text_color(s_aqi_layer, get_foreground_color());
  text_layer_set_text_color(s_tide_layer, get_foreground_color());
  text_layer_set_text_color(s_pollen_layer, get_background_color());  // Inverted for header
  text_layer_set_text_color(s_precip_layer, get_background_color());  // Inverted for header
  text_layer_set_text_color(s_sunrise_layer, get_foreground_color());
  text_layer_set_text_color(s_sunset_layer, get_foreground_color());

  // Reload weather icons with inverted versions
  if (s_weather_icon) {
    gbitmap_destroy(s_weather_icon);
    uint32_t icon_resource = get_weather_icon_resource(s_weather_data.weather_code, true);
    s_weather_icon = gbitmap_create_with_resource(icon_resource);
    bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon);
  }
  if (s_weather_icon_tomorrow) {
    gbitmap_destroy(s_weather_icon_tomorrow);
    uint32_t icon_resource_tomorrow = get_weather_icon_resource(s_weather_data.weather_code_tomorrow, false);
    s_weather_icon_tomorrow = gbitmap_create_with_resource(icon_resource_tomorrow);
    bitmap_layer_set_bitmap(s_weather_icon_tomorrow_layer, s_weather_icon_tomorrow);
  }

  // Reload sunrise/sunset arrow bitmaps with inverted versions
  if (s_arrow_up_bitmap) {
    gbitmap_destroy(s_arrow_up_bitmap);
    s_arrow_up_bitmap = gbitmap_create_with_resource(
      s_config.invert_colors ? RESOURCE_ID_ARROW_UP_INVERTED : RESOURCE_ID_ARROW_UP
    );
    bitmap_layer_set_bitmap(s_arrow_up_layer, s_arrow_up_bitmap);
  }
  if (s_arrow_down_bitmap) {
    gbitmap_destroy(s_arrow_down_bitmap);
    s_arrow_down_bitmap = gbitmap_create_with_resource(
      s_config.invert_colors ? RESOURCE_ID_ARROW_DOWN_INVERTED : RESOURCE_ID_ARROW_DOWN
    );
    bitmap_layer_set_bitmap(s_arrow_down_layer, s_arrow_down_bitmap);
  }
  if (s_wave_bitmap) {
    gbitmap_destroy(s_wave_bitmap);
    s_wave_bitmap = gbitmap_create_with_resource(
      s_config.invert_colors ? RESOURCE_ID_WAVE_INVERTED : RESOURCE_ID_WAVE
    );
    bitmap_layer_set_bitmap(s_wave_layer, s_wave_bitmap);
  }

  // Force divider layer redraw (header and lines)
  layer_mark_dirty(s_divider_layer);
}

// Window load handler
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Set window background color based on inversion setting
  window_set_background_color(window, get_background_color());

  // Screen is 144x168
  // 3x3 Grid Layout (all boxes equal size: 48px × 28px):
  // 0-20: Header [location | icon | temp] (black bar)
  // 22-78: Time box [date | TIME]
  // 84-112: Row 1 [Lo | Current | Hi] (temp grid)
  // 112-140: Row 2 [Wind | UV | AQI] (data grid)
  // 140-168: Row 3 [Tide | Sunrise | Sunset] (time grid)

  // Create divider layer (full screen, drawn behind text)
  s_divider_layer = layer_create(bounds);
  layer_set_update_proc(s_divider_layer, divider_layer_update_proc);
  layer_add_child(window_layer, s_divider_layer);

  // === HEADER SECTION (0-20) ===
  // Location (centered) - WHITE text on BLACK background
  s_location_layer = create_text_layer_colored(
    GRect(0, 3, bounds.size.w, 16),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
    get_background_color(),  // Inverted: white on black header, or black on white header
    GColorClear
  );
  layer_add_child(window_layer, text_layer_get_layer(s_location_layer));

  // Precipitation probability (top left corner of header) - inverted colors for black header
  s_precip_layer = create_text_layer_colored(
    GRect(4, 3, 30, 14),
    GTextAlignmentLeft,
    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
    get_background_color(),  // Inverted: white text on black header, or black on white header
    GColorClear
  );
  layer_add_child(window_layer, text_layer_get_layer(s_precip_layer));
  text_layer_set_text(s_precip_layer, "");  // Empty by default

  // === TIME SECTION (26-80) ===
  // Date (centered) - smaller, above time (add this FIRST so icons appear on top)
  s_date_layer = create_text_layer(
    GRect(0, 28, bounds.size.w, 14),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_14)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Weather icon (top-left corner of time box)
  s_weather_icon_layer = bitmap_layer_create(GRect(6, 28, 20, 20));
  bitmap_layer_set_background_color(s_weather_icon_layer, GColorClear);
  bitmap_layer_set_compositing_mode(s_weather_icon_layer, get_bitmap_compositing_mode());
  layer_add_child(window_layer, bitmap_layer_get_layer(s_weather_icon_layer));

  // Tomorrow's weather icon (top-right corner of time box)
  s_weather_icon_tomorrow_layer = bitmap_layer_create(GRect(118, 28, 20, 20));
  bitmap_layer_set_background_color(s_weather_icon_tomorrow_layer, GColorClear);
  bitmap_layer_set_compositing_mode(s_weather_icon_tomorrow_layer, get_bitmap_compositing_mode());
  layer_add_child(window_layer, bitmap_layer_get_layer(s_weather_icon_tomorrow_layer));

  // Extra large time display (centered) - use biggest font
  s_time_layer = create_text_layer(
    GRect(0, 34, bounds.size.w, 42),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Pollen display (top right corner, next to city name) - inverted colors for black header
  s_pollen_layer = create_text_layer_colored(
    GRect(110, 3, 30, 14),
    GTextAlignmentRight,
    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
    get_background_color(),  // Inverted: white text on black header, or black on white header
    GColorClear
  );
  layer_add_child(window_layer, text_layer_get_layer(s_pollen_layer));
  text_layer_set_text(s_pollen_layer, "");  // Empty by default

  // === ALERT SECTION (80-96, only shown when active) ===
  s_alert_layer = create_text_layer(
    GRect(0, 80, bounds.size.w, 16),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_alert_layer));
  layer_set_hidden(text_layer_get_layer(s_alert_layer), !s_alert_active);

  // === MUNI BUS COUNTDOWN (Top-left grid cell: 0-48, 98-120) ===
  s_muni_layer = create_text_layer(
    GRect(-2, 96, 60, 20),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_18)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_muni_layer));
  text_layer_set_text(s_muni_layer, ":)");  // Default placeholder

  // === TEMPERATURE GRID (Row 1: 96-120, 24px boxes anchored to bottom) ===
  // Low temp - unused (combined with high in s_temp_max_layer)
  s_temp_min_layer = create_text_layer(
    GRect(0, 0, 0, 0),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_14)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_temp_min_layer));
  layer_set_hidden(text_layer_get_layer(s_temp_min_layer), true);

  // Current - Middle column (48-96) - centered in full 48px box, shifted right 4px, up 2px
  s_temp_current_layer = create_text_layer(
    GRect(52, 96, 48, 20),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_18)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_temp_current_layer));

  // High/Low combined - Right column (96-144) - format: "53°|59°" centered
  s_temp_max_layer = create_text_layer(
    GRect(96, 100, 48, 20),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_14)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_temp_max_layer));

  // Temperature arrows hidden (removed from display)
  s_temp_arrow_low_layer = NULL;
  s_temp_arrow_low_bitmap = NULL;
  s_temp_arrow_high_layer = NULL;
  s_temp_arrow_high_bitmap = NULL;

  // === DATA GRID (Row 2: 120-144, 24px boxes anchored to bottom) ===
  // Wind - Left column (0-48) - centered in 24px box
  s_wind_layer = create_text_layer(
    GRect(0, 122, 48, 20),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_18)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_wind_layer));

  // UV - Middle column (48-96) - centered in 24px box
  s_uv_layer = create_text_layer(
    GRect(48, 122, 48, 20),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_18)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_uv_layer));

  // AQI - Right column (96-144) - centered in 24px box
  s_aqi_layer = create_text_layer(
    GRect(96, 122, 48, 20),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_18)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_aqi_layer));

  // === TIME GRID (Row 3: 144-168, 24px boxes anchored to bottom) ===
  // Tide - Left column (0-48) - H/L + time, centered
  s_tide_layer = create_text_layer(
    GRect(0, 148, 48, 18),
    GTextAlignmentCenter,
    fonts_get_system_font(FONT_KEY_GOTHIC_14)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_tide_layer));

  // Load wave bitmap (hidden, kept for compatibility)
  s_wave_bitmap = gbitmap_create_with_resource(
    s_config.invert_colors ? RESOURCE_ID_WAVE_INVERTED : RESOURCE_ID_WAVE
  );
  s_wave_layer = bitmap_layer_create(GRect(6, 149, 10, 10));
  bitmap_layer_set_bitmap(s_wave_layer, s_wave_bitmap);
  bitmap_layer_set_compositing_mode(s_wave_layer, get_bitmap_compositing_mode());
  layer_add_child(window_layer, bitmap_layer_get_layer(s_wave_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_wave_layer), true);

  // Sunrise - Middle column (48-96) with arrow bitmap
  // Load arrow up bitmap
  s_arrow_up_bitmap = gbitmap_create_with_resource(
    s_config.invert_colors ? RESOURCE_ID_ARROW_UP_INVERTED : RESOURCE_ID_ARROW_UP
  );
  s_arrow_up_layer = bitmap_layer_create(GRect(54, 149, 10, 10));
  bitmap_layer_set_bitmap(s_arrow_up_layer, s_arrow_up_bitmap);
  bitmap_layer_set_compositing_mode(s_arrow_up_layer, get_bitmap_compositing_mode());
  layer_add_child(window_layer, bitmap_layer_get_layer(s_arrow_up_layer));

  s_sunrise_layer = create_text_layer(
    GRect(64, 148, 32, 18),
    GTextAlignmentLeft,
    fonts_get_system_font(FONT_KEY_GOTHIC_14)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_sunrise_layer));

  // Sunset - Right column (96-144) with arrow bitmap
  // Load arrow down bitmap
  s_arrow_down_bitmap = gbitmap_create_with_resource(
    s_config.invert_colors ? RESOURCE_ID_ARROW_DOWN_INVERTED : RESOURCE_ID_ARROW_DOWN
  );
  s_arrow_down_layer = bitmap_layer_create(GRect(102, 149, 10, 10));
  bitmap_layer_set_bitmap(s_arrow_down_layer, s_arrow_down_bitmap);
  bitmap_layer_set_compositing_mode(s_arrow_down_layer, get_bitmap_compositing_mode());
  layer_add_child(window_layer, bitmap_layer_get_layer(s_arrow_down_layer));

  s_sunset_layer = create_text_layer(
    GRect(112, 148, 32, 18),
    GTextAlignmentLeft,
    fonts_get_system_font(FONT_KEY_GOTHIC_14)
  );
  layer_add_child(window_layer, text_layer_get_layer(s_sunset_layer));

  // Initial display update (config and data already loaded in init())
  update_time();
  update_weather_display();

  // Request fresh weather data
  request_weather_update();
}

// Window unload handler
static void main_window_unload(Window *window) {
  // Destroy divider layer
  layer_destroy(s_divider_layer);

  // Destroy weather icon
  if (s_weather_icon) {
    gbitmap_destroy(s_weather_icon);
  }
  bitmap_layer_destroy(s_weather_icon_layer);

  // Destroy tomorrow's weather icon
  if (s_weather_icon_tomorrow) {
    gbitmap_destroy(s_weather_icon_tomorrow);
  }
  bitmap_layer_destroy(s_weather_icon_tomorrow_layer);

  // Destroy temperature arrow bitmaps and layers (if they exist)
  if (s_temp_arrow_low_bitmap) {
    gbitmap_destroy(s_temp_arrow_low_bitmap);
  }
  if (s_temp_arrow_low_layer) {
    bitmap_layer_destroy(s_temp_arrow_low_layer);
  }

  if (s_temp_arrow_high_bitmap) {
    gbitmap_destroy(s_temp_arrow_high_bitmap);
  }
  if (s_temp_arrow_high_layer) {
    bitmap_layer_destroy(s_temp_arrow_high_layer);
  }

  // Destroy arrow and wave bitmaps and layers
  gbitmap_destroy(s_arrow_up_bitmap);
  gbitmap_destroy(s_arrow_down_bitmap);
  gbitmap_destroy(s_wave_bitmap);
  bitmap_layer_destroy(s_arrow_up_layer);
  bitmap_layer_destroy(s_arrow_down_layer);
  bitmap_layer_destroy(s_wave_layer);

  // Destroy text layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_alert_layer);
  text_layer_destroy(s_location_layer);
  text_layer_destroy(s_muni_layer);
  text_layer_destroy(s_temp_min_layer);
  text_layer_destroy(s_temp_current_layer);
  text_layer_destroy(s_temp_max_layer);
  text_layer_destroy(s_wind_layer);
  text_layer_destroy(s_uv_layer);
  text_layer_destroy(s_aqi_layer);
  text_layer_destroy(s_tide_layer);
  text_layer_destroy(s_pollen_layer);
  text_layer_destroy(s_precip_layer);
  text_layer_destroy(s_sunrise_layer);
  text_layer_destroy(s_sunset_layer);
}

// Initialize app
static void init() {
  // Load persisted config and data BEFORE creating UI
  load_config();
  load_persisted_data();

  // Create main window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Clear any stale alerts on init
  s_weather_data.alert_active = false;
  s_weather_data.alert_text[0] = '\0';

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks for AppMessage
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage with large buffer
  const int inbox_size = 512;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

// Deinitialize app
static void deinit() {
  window_destroy(s_main_window);
}

// Main entry point
int main(void) {
  init();
  app_event_loop();
  deinit();
}
