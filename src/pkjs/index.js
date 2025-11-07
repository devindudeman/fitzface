// FitzFace - JavaScript Companion App
// Fetches weather, AQI, and tide data for Pebble watchface

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

// Configuration
var CONFIG = {
  TIDE_STATION: '9414290', // Default: San Francisco
  TEMP_UNIT: 'F',
  SHOW_AQI: true,
  SHOW_UV: true,
  SHOW_WIND: true,
  SHOW_TIDE: true,
  SHOW_SUNRISE: true,
  INVERT: false,
  MUNI_ENABLED: false,
  MUNI_API_KEY: '',
  MUNI_STOP_CODE: '',
  MUNI_ROUTE: '',
  MUNI_DIRECTION: 'IB',
  POLLEN_ENABLED: false,
  POLLEN_API_KEY: ''
};

// Cache for location
var lastLocation = null;

// Load configuration from localStorage
function loadConfig() {
  var stored = localStorage.getItem('fitzface_config');
  if (stored) {
    try {
      var config = JSON.parse(stored);
      CONFIG = Object.assign(CONFIG, config);
      console.log('Loaded config from storage:', JSON.stringify(CONFIG));
    } catch (e) {
      console.log('Error loading config: ' + e);
    }
  } else {
    console.log('No stored config, using defaults:', JSON.stringify(CONFIG));
  }
}

// Save configuration to localStorage
function saveConfig() {
  localStorage.setItem('fitzface_config', JSON.stringify(CONFIG));
}

// Get location using Geolocation API
function getLocation(callback) {
  console.log('Requesting location...');

  navigator.geolocation.getCurrentPosition(
    function(pos) {
      var location = {
        lat: pos.coords.latitude,
        lon: pos.coords.longitude
      };
      console.log('Location acquired: ' + location.lat + ', ' + location.lon);
      lastLocation = location;
      callback(location);
    },
    function(err) {
      console.log('Location error: ' + err.message);
      // Use cached location if available
      if (lastLocation) {
        console.log('Using cached location');
        callback(lastLocation);
      } else {
        callback(null);
      }
    },
    {
      timeout: 15000,
      maximumAge: 60000
    }
  );
}

// Fetch weather data from Open-Meteo
function fetchWeather(location, callback) {
  var tempUnit = CONFIG.TEMP_UNIT === 'C' ? 'celsius' : 'fahrenheit';
  console.log('Fetching weather with temperature unit:', tempUnit, '(CONFIG.TEMP_UNIT=' + CONFIG.TEMP_UNIT + ')');

  var url = 'https://api.open-meteo.com/v1/forecast?' +
    'latitude=' + location.lat +
    '&longitude=' + location.lon +
    '&current=temperature_2m,wind_speed_10m,weather_code,uv_index' +
    '&hourly=precipitation_probability,precipitation,wind_gusts_10m,weather_code,temperature_2m,uv_index' +
    '&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,wind_speed_10m_max,weather_code,precipitation_probability_max,wind_gusts_10m_max' +
    '&temperature_unit=' + tempUnit +
    '&wind_speed_unit=mph' +
    '&precipitation_unit=inch' +
    '&timezone=auto' +
    '&forecast_days=2' +
    '&forecast_hours=24';

  console.log('Fetching weather from Open-Meteo...');

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = 15000;

  xhr.onload = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log('Weather data received');
          callback(null, response);
        } catch (e) {
          console.log('Error parsing weather response: ' + e);
          callback(e, null);
        }
      } else {
        console.log('Weather request failed: ' + xhr.status);
        callback(new Error('Request failed: ' + xhr.status), null);
      }
    }
  };

  xhr.onerror = function() {
    console.log('Weather request error');
    callback(new Error('Network error'), null);
  };

  xhr.ontimeout = function() {
    console.log('Weather request timeout');
    callback(new Error('Timeout'), null);
  };

  xhr.send();
}

// Fetch AQI data from Open-Meteo Air Quality API
function fetchAQI(location, callback) {
  if (!CONFIG.SHOW_AQI) {
    callback(null, { aqi: 0 });
    return;
  }

  var url = 'https://air-quality-api.open-meteo.com/v1/air-quality?' +
    'latitude=' + location.lat +
    '&longitude=' + location.lon +
    '&current=us_aqi' +
    '&timezone=auto';

  console.log('Fetching AQI data...');

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = 15000;

  xhr.onload = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log('AQI data received: ' + response.current.us_aqi);
          callback(null, { aqi: Math.round(response.current.us_aqi || 0) });
        } catch (e) {
          console.log('Error parsing AQI response: ' + e);
          callback(null, { aqi: 0 });
        }
      } else {
        console.log('AQI request failed: ' + xhr.status);
        callback(null, { aqi: 0 });
      }
    }
  };

  xhr.onerror = function() {
    console.log('AQI request error');
    callback(null, { aqi: 0 });
  };

  xhr.ontimeout = function() {
    console.log('AQI request timeout');
    callback(null, { aqi: 0 });
  };

  xhr.send();
}

// Fetch tide data from NOAA
function fetchTides(callback) {
  if (!CONFIG.SHOW_TIDE || !CONFIG.TIDE_STATION) {
    callback(null, null);
    return;
  }

  var now = new Date();
  var tomorrow = new Date(now.getTime() + 48 * 60 * 60 * 1000);

  var beginDate = formatNOAADate(now);
  var endDate = formatNOAADate(tomorrow);

  var url = 'https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?' +
    'product=predictions' +
    '&application=FitzFace' +
    '&begin_date=' + beginDate +
    '&end_date=' + endDate +
    '&datum=MLLW' +
    '&station=' + CONFIG.TIDE_STATION +
    '&time_zone=lst_ldt' +
    '&units=english' +
    '&interval=hilo' +
    '&format=json';

  console.log('Fetching tide data from NOAA...');

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = 15000;

  xhr.onload = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          if (response.predictions && response.predictions.length > 0) {
            // Find next tide (first prediction after now)
            var now = new Date();
            var nextTide = null;

            for (var i = 0; i < response.predictions.length; i++) {
              var tideTime = new Date(response.predictions[i].t);
              if (tideTime > now) {
                nextTide = response.predictions[i];
                break;
              }
            }

            if (nextTide) {
              console.log('Next tide: ' + nextTide.type + ' at ' + nextTide.t);
              callback(null, nextTide);
            } else {
              console.log('No future tide predictions available');
              callback(null, null);
            }
          } else {
            console.log('No tide predictions available');
            callback(null, null);
          }
        } catch (e) {
          console.log('Error parsing tide response: ' + e);
          callback(null, null);
        }
      } else {
        console.log('Tide request failed: ' + xhr.status);
        callback(null, null);
      }
    }
  };

  xhr.onerror = function() {
    console.log('Tide request error');
    callback(null, null);
  };

  xhr.ontimeout = function() {
    console.log('Tide request timeout');
    callback(null, null);
  };

  xhr.send();
}

// Format date for NOAA API (YYYYMMDD)
function formatNOAADate(date) {
  var year = date.getFullYear();
  var month = ('0' + (date.getMonth() + 1)).slice(-2);
  var day = ('0' + date.getDate()).slice(-2);
  return year + month + day;
}

// Parse NOAA tide time to Unix timestamp
function parseTideTime(timeStr) {
  // Format: "YYYY-MM-DD HH:MM"
  var date = new Date(timeStr);
  return Math.floor(date.getTime() / 1000);
}

// Parse Open-Meteo ISO timestamp to Unix timestamp
function parseISOTime(isoStr) {
  var date = new Date(isoStr);
  return Math.floor(date.getTime() / 1000);
}

// Reverse geocode to get location name
function getLocationName(location, callback) {
  // Use Nominatim reverse geocoding API
  var url = 'https://nominatim.openstreetmap.org/reverse?' +
    'lat=' + location.lat +
    '&lon=' + location.lon +
    '&format=json' +
    '&zoom=10';  // City level

  console.log('Fetching city name from Nominatim...');

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = 10000;
  xhr.setRequestHeader('User-Agent', 'FitzFace Pebble Watchface');

  xhr.onload = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          // Try to get city, town, or village name
          var city = response.address.city ||
                     response.address.town ||
                     response.address.village ||
                     response.address.county ||
                     'Unknown';
          console.log('City name: ' + city);
          callback(city);
        } catch (e) {
          console.log('Error parsing geocoding response: ' + e);
          callback(location.lat.toFixed(1) + '°, ' + location.lon.toFixed(1) + '°');
        }
      } else {
        console.log('Geocoding request failed: ' + xhr.status);
        callback(location.lat.toFixed(1) + '°, ' + location.lon.toFixed(1) + '°');
      }
    }
  };

  xhr.onerror = function() {
    console.log('Geocoding request error');
    callback(location.lat.toFixed(1) + '°, ' + location.lon.toFixed(1) + '°');
  };

  xhr.ontimeout = function() {
    console.log('Geocoding request timeout');
    callback(location.lat.toFixed(1) + '°, ' + location.lon.toFixed(1) + '°');
  };

  xhr.send();
}

// Fetch MUNI bus predictions from 511.org API
function fetchMuniBusPredictions(callback) {
  if (!CONFIG.MUNI_ENABLED || !CONFIG.MUNI_API_KEY || !CONFIG.MUNI_STOP_CODE || !CONFIG.MUNI_ROUTE) {
    console.log('MUNI tracking disabled or not configured');
    callback(null);
    return;
  }

  var url = 'http://api.511.org/transit/StopMonitoring?' +
    'api_key=' + encodeURIComponent(CONFIG.MUNI_API_KEY) +
    '&agency=SF' +
    '&stopCode=' + encodeURIComponent(CONFIG.MUNI_STOP_CODE) +
    '&format=json';

  console.log('Fetching MUNI predictions for route ' + CONFIG.MUNI_ROUTE + ' at stop ' + CONFIG.MUNI_STOP_CODE);

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = 10000;

  xhr.onload = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          var arrivals = parseMuniPredictions(response);
          console.log('MUNI predictions received: ' + JSON.stringify(arrivals));
          callback(arrivals);
        } catch (e) {
          console.log('Error parsing MUNI response: ' + e);
          callback(null);
        }
      } else {
        console.log('MUNI request failed: ' + xhr.status + ' - ' + xhr.responseText);
        callback(null);
      }
    }
  };

  xhr.onerror = function() {
    console.log('MUNI request error');
    callback(null);
  };

  xhr.ontimeout = function() {
    console.log('MUNI request timeout');
    callback(null);
  };

  xhr.send();
}

// Parse MUNI 511.org API response
function parseMuniPredictions(response) {
  try {
    var visits = response.ServiceDelivery.StopMonitoringDelivery.MonitoredStopVisit;
    if (!visits || visits.length === 0) {
      console.log('No MUNI predictions available');
      return null;
    }

    var responseTime = new Date(response.ServiceDelivery.ResponseTimestamp);
    var arrivals = [];
    var targetRoute = CONFIG.MUNI_ROUTE.toUpperCase();
    var targetDirection = CONFIG.MUNI_DIRECTION || 'IB';

    // Filter by route and direction, calculate minutes
    for (var i = 0; i < visits.length; i++) {
      var journey = visits[i].MonitoredVehicleJourney;
      var lineRef = journey.LineRef;
      var directionRef = journey.DirectionRef || '';

      // Match route and direction
      if (lineRef === targetRoute && directionRef.indexOf(targetDirection) !== -1) {
        // Try multiple possible time fields (511.org API uses different field names)
        var expectedTime = journey.MonitoredCall.ExpectedArrivalTime ||
                          journey.MonitoredCall.AimedArrivalTime ||
                          journey.MonitoredCall.ExpectedDepartureTime ||
                          journey.MonitoredCall.AimedDepartureTime;
        if (expectedTime) {
          var arrivalTime = new Date(expectedTime);
          var minutes = Math.floor((arrivalTime - responseTime) / 60000);
          if (minutes >= 0) {  // Only future arrivals
            arrivals.push(minutes);
          }
        }
      }
    }

    // Sort by time
    arrivals.sort(function(a, b) { return a - b; });

    if (arrivals.length === 0) {
      console.log('No matching MUNI arrivals found for ' + targetRoute + ' ' + targetDirection);
      return null;
    }

    console.log('Found ' + arrivals.length + ' MUNI arrivals: ' + arrivals.join(', '));
    return {
      next1: arrivals[0] !== undefined ? arrivals[0] : -1,
      next2: arrivals[1] !== undefined ? arrivals[1] : -1
    };
  } catch (e) {
    console.log('Error parsing MUNI predictions: ' + e);
    return null;
  }
}

// Fetch pollen data from Google Pollen API
function fetchPollen(location, callback) {
  if (!CONFIG.POLLEN_ENABLED || !CONFIG.POLLEN_API_KEY) {
    console.log('Pollen disabled or no API key');
    callback(null);
    return;
  }

  var apiKey = CONFIG.POLLEN_API_KEY.trim();
  if (!apiKey) {
    console.log('Pollen API key is empty');
    callback(null);
    return;
  }

  var url = 'https://pollen.googleapis.com/v1/forecast:lookup?' +
    'key=' + apiKey +
    '&location.latitude=' + location.lat +
    '&location.longitude=' + location.lon +
    '&days=1';

  console.log('Fetching pollen data from Google Pollen API...');

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = 10000;

  xhr.onload = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          var pollenData = parsePollenResponse(response);
          console.log('Pollen data received: ' + JSON.stringify(pollenData));
          callback(pollenData);
        } catch (e) {
          console.log('Error parsing pollen response: ' + e);
          callback(null);
        }
      } else {
        console.log('Pollen request failed: ' + xhr.status);
        callback(null);
      }
    }
  };

  xhr.onerror = function() {
    console.log('Pollen request error');
    callback(null);
  };

  xhr.ontimeout = function() {
    console.log('Pollen request timeout');
    callback(null);
  };

  xhr.send();
}

// Parse Google Pollen API response
function parsePollenResponse(response) {
  try {
    if (!response.dailyInfo || response.dailyInfo.length === 0) {
      console.log('No pollen data available');
      return null;
    }

    var today = response.dailyInfo[0];
    if (!today.pollenTypeInfo) {
      console.log('No pollen type info');
      return null;
    }

    var tree = 0, grass = 0, weed = 0;

    // Extract pollen indices for each type
    for (var i = 0; i < today.pollenTypeInfo.length; i++) {
      var pollenType = today.pollenTypeInfo[i];
      var value = pollenType.indexInfo ? pollenType.indexInfo.value : 0;

      if (pollenType.code === 'TREE') {
        tree = value;
      } else if (pollenType.code === 'GRASS') {
        grass = value;
      } else if (pollenType.code === 'WEED') {
        weed = value;
      }
    }

    console.log('Parsed pollen: Tree=' + tree + ', Grass=' + grass + ', Weed=' + weed);

    return {
      tree: tree,
      grass: grass,
      weed: weed
    };
  } catch (e) {
    console.log('Error parsing pollen data: ' + e);
    return null;
  }
}

// Fetch all data and send to watch
function updateWeather() {
  console.log('Starting weather update...');

  getLocation(function(location) {
    if (!location) {
      console.log('Failed to get location');
      return;
    }

    // Fetch weather, AQI, tides, MUNI, and pollen in parallel
    var weatherData = null;
    var aqiData = null;
    var tideData = null;
    var muniData = null;
    var pollenData = null;
    var completed = 0;
    var total = 5;

    function checkComplete() {
      completed++;
      if (completed === total) {
        sendDataToWatch(location, weatherData, aqiData, tideData, muniData, pollenData);
      }
    }

    // Fetch weather
    fetchWeather(location, function(err, data) {
      if (!err && data) {
        weatherData = data;
      }
      checkComplete();
    });

    // Fetch AQI
    fetchAQI(location, function(err, data) {
      if (!err && data) {
        aqiData = data;
      }
      checkComplete();
    });

    // Fetch tides
    fetchTides(function(err, data) {
      if (!err) {
        tideData = data;
      }
      checkComplete();
    });

    // Fetch MUNI predictions
    fetchMuniBusPredictions(function(data) {
      if (data) {
        muniData = data;
      }
      checkComplete();
    });

    // Fetch pollen data
    fetchPollen(location, function(data) {
      if (data) {
        pollenData = data;
      }
      checkComplete();
    });
  });
}

// Detect weather alerts from hourly forecast data
function detectWeatherAlerts(weatherData, aqiData, pollenData) {
  if (!weatherData || !weatherData.hourly) {
    return { active: false, text: '' };
  }

  var hourly = weatherData.hourly;
  var now = new Date();
  var alerts = [];

  // Check next 24 hours for unusual conditions
  for (var i = 0; i < Math.min(24, hourly.time.length); i++) {
    var hour = new Date(hourly.time[i]);
    var code = hourly.weather_code[i];
    var precip = hourly.precipitation_probability[i];
    var windGust = hourly.wind_gusts_10m[i];
    var uvIndex = hourly.uv_index ? hourly.uv_index[i] : 0;

    // Skip past hours
    if (hour < now) continue;

    // Priority 1: Thunderstorms (95-99)
    if (code >= 95 && code <= 99) {
      alerts.push({
        priority: 1,
        text: 'Thunderstorm',
        startHour: hour
      });
    }

    // Priority 2: Freezing conditions (56-57, 66-67)
    else if ((code >= 56 && code <= 57) || (code >= 66 && code <= 67)) {
      alerts.push({
        priority: 2,
        text: code >= 66 ? 'Freezing Rain' : 'Freezing Drizzle',
        startHour: hour
      });
    }

    // Priority 3: Snow (71-77, 85-86)
    else if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) {
      var snowType = code === 75 || code === 77 || code === 86 ? 'Heavy Snow' : 'Snow';
      alerts.push({
        priority: 3,
        text: snowType,
        startHour: hour
      });
    }

    // Priority 4: Heavy rain (63-65, 81-82)
    else if ((code >= 63 && code <= 65) || (code >= 81 && code <= 82)) {
      alerts.push({
        priority: 4,
        text: 'Heavy Rain',
        startHour: hour
      });
    }

    // Priority 5: Fog (45, 48)
    else if (code === 45 || code === 48) {
      alerts.push({
        priority: 5,
        text: 'Fog',
        startHour: hour
      });
    }

    // Priority 5.5: High UV Index (≥8 = Very High)
    if (uvIndex >= 8) {
      alerts.push({
        priority: 5.5,
        text: uvIndex >= 11 ? 'Extreme UV' : 'High UV',
        startHour: hour
      });
    }

    // Priority 6: High wind gusts (≥20mph)
    if (windGust >= 20) {
      alerts.push({
        priority: 6,
        text: 'Wind ' + Math.round(windGust) + 'mph',
        startHour: hour
      });
    }

    // Priority 7: High precipitation probability (≥30%)
    if (precip >= 30) {
      alerts.push({
        priority: 7,
        text: 'Rain ' + Math.round(precip) + '%',
        startHour: hour
      });
    }
  }

  // Check current AQI (priority 5.5, same as UV)
  if (aqiData && aqiData.aqi >= 100) {
    alerts.push({
      priority: 5.5,
      text: aqiData.aqi >= 150 ? 'Unhealthy AQI' : 'High AQI',
      startHour: now
    });
  }

  // Check current pollen (priority 5.5, same as UV/AQI)
  if (pollenData) {
    var maxPollen = Math.max(pollenData.tree, pollenData.grass, pollenData.weed);
    if (maxPollen >= 4) {
      var pollenType = '';
      var pollenValue = maxPollen;

      // Find which pollen type is highest
      if (pollenData.tree >= 4 && pollenData.tree === maxPollen) {
        pollenType = 'Tree';
      } else if (pollenData.grass >= 4 && pollenData.grass === maxPollen) {
        pollenType = 'Grass';
      } else if (pollenData.weed >= 4 && pollenData.weed === maxPollen) {
        pollenType = 'Weed';
      }

      if (pollenType) {
        alerts.push({
          priority: 5.5,
          text: (pollenValue === 5 ? 'Very High ' : 'High ') + pollenType + ' Pollen',
          startHour: now
        });
      }
    }
  }

  // No alerts found
  if (alerts.length === 0) {
    return { active: false, text: '' };
  }

  // Sort by priority and get highest priority alert
  alerts.sort(function(a, b) {
    return a.priority - b.priority;
  });

  var topAlert = alerts[0];

  // Find time range for this alert type
  var startHour = topAlert.startHour;
  var endHour = startHour;

  for (var j = 0; j < alerts.length; j++) {
    if (alerts[j].text === topAlert.text && alerts[j].priority === topAlert.priority) {
      var alertHour = alerts[j].startHour;
      if (alertHour > endHour) {
        endHour = alertHour;
      }
    }
  }

  // Format time range
  var startTime = formatHour(startHour);
  var endTime = formatHour(new Date(endHour.getTime() + 60 * 60 * 1000)); // +1 hour
  var timeRange = startTime + '-' + endTime;

  // Build alert message
  var alertText = topAlert.text + ' ' + timeRange;

  console.log('Weather alert detected: ' + alertText);

  return {
    active: true,
    text: alertText
  };
}

// Format hour for display (e.g., "3PM", "11AM")
function formatHour(date) {
  var hours = date.getHours();
  var ampm = hours >= 12 ? 'PM' : 'AM';
  hours = hours % 12;
  hours = hours ? hours : 12; // 0 -> 12
  return hours + ampm;
}

// Send data to watch via AppMessage
function sendDataToWatch(location, weatherData, aqiData, tideData, muniData, pollenData) {
  console.log('Preparing data to send to watch...');

  var message = {};

  // Weather data
  if (weatherData && weatherData.current) {
    message.TEMPERATURE = Math.round(weatherData.current.temperature_2m);
    message.WIND_SPEED = Math.round(weatherData.current.wind_speed_10m);
    message.UV_INDEX = Math.round(weatherData.current.uv_index || 0);
    message.WEATHER_CODE = weatherData.current.weather_code || 0;
  }

  if (weatherData && weatherData.daily) {
    message.TEMP_MAX = Math.round(weatherData.daily.temperature_2m_max[0]);
    message.TEMP_MIN = Math.round(weatherData.daily.temperature_2m_min[0]);
    message.SUNRISE = parseISOTime(weatherData.daily.sunrise[0]);
    message.SUNSET = parseISOTime(weatherData.daily.sunset[0]);
    message.WIND_MAX = Math.round(weatherData.daily.wind_speed_10m_max[0]);
    // Tomorrow's weather code (day 1)
    if (weatherData.daily.weather_code && weatherData.daily.weather_code.length > 1) {
      message.WEATHER_CODE_TOMORROW = weatherData.daily.weather_code[1] || 0;
    }
  }

  // AQI data
  if (aqiData) {
    message.AQI = aqiData.aqi;
  }

  // Tide data
  if (tideData) {
    message.TIDE_NEXT_TIME = parseTideTime(tideData.t);
    message.TIDE_NEXT_TYPE = tideData.type === 'H' ? 1 : 0; // 1 = high, 0 = low
    message.TIDE_NEXT_HEIGHT = Math.round(parseFloat(tideData.v) * 10) / 10;
  }

  // Weather alerts (includes UV, AQI, and pollen alerts)
  var alert = detectWeatherAlerts(weatherData, aqiData, pollenData);
  message.ALERT_ACTIVE = alert.active ? 1 : 0;
  message.ALERT_TEXT = alert.text || '';

  // Location name
  getLocationName(location, function(name) {
    message.LOCATION_NAME = name;

    // Configuration
    message.CONFIG_TEMP_UNIT = CONFIG.TEMP_UNIT === 'C' ? 1 : 0;
    message.CONFIG_SHOW_AQI = CONFIG.SHOW_AQI ? 1 : 0;
    message.CONFIG_SHOW_UV = CONFIG.SHOW_UV ? 1 : 0;
    message.CONFIG_SHOW_WIND = CONFIG.SHOW_WIND ? 1 : 0;
    message.CONFIG_SHOW_TIDE = CONFIG.SHOW_TIDE ? 1 : 0;
    message.CONFIG_SHOW_SUNRISE = CONFIG.SHOW_SUNRISE ? 1 : 0;
    message.CONFIG_INVERT = CONFIG.INVERT ? 1 : 0;

  // MUNI data
  if (muniData) {
    message.MUNI_NEXT_1 = muniData.next1;
    message.MUNI_NEXT_2 = muniData.next2;
    console.log('MUNI predictions added to message: ' + muniData.next1 + ', ' + muniData.next2);
  } else {
    message.MUNI_NEXT_1 = -1;  // -1 indicates no data
    message.MUNI_NEXT_2 = -1;
  }

  // Pollen data
  if (pollenData) {
    message.POLLEN_TREE = pollenData.tree;
    message.POLLEN_GRASS = pollenData.grass;
    message.POLLEN_WEED = pollenData.weed;
    console.log('Pollen data added: Tree=' + pollenData.tree + ', Grass=' + pollenData.grass + ', Weed=' + pollenData.weed);
  } else {
    message.POLLEN_TREE = -1;  // -1 indicates no data
    message.POLLEN_GRASS = -1;
    message.POLLEN_WEED = -1;
  }

    // Send to watch
    console.log('Sending message to watch:', JSON.stringify(message));
    Pebble.sendAppMessage(message,
      function(e) {
        console.log('Message sent successfully');
      },
      function(e) {
        console.log('Error sending message: ' + JSON.stringify(e));
      }
    );
  });
}

// Pebble event handlers
Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
  loadConfig();
  updateWeather();
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received from watch');
  // Watch is requesting update
  updateWeather();
});

Pebble.addEventListener('showConfiguration', function(e) {
  console.log('Showing configuration');
  // Clay will handle this
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Configuration window closed');

  if (e && !e.response) {
    console.log('No configuration data returned');
    return;
  }

  // Get the config data from Clay
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration received');

  // Update CONFIG object (Clay sends values wrapped in {value: X})
  if (configData.TIDE_STATION) {
    CONFIG.TIDE_STATION = configData.TIDE_STATION.value;
  }
  if (configData.CONFIG_TEMP_UNIT) {
    console.log('Temperature unit changed from', CONFIG.TEMP_UNIT, 'to', configData.CONFIG_TEMP_UNIT.value);
    CONFIG.TEMP_UNIT = configData.CONFIG_TEMP_UNIT.value;
  }
  if (configData.SHOW_AQI !== undefined) {
    CONFIG.SHOW_AQI = configData.SHOW_AQI.value;
  }
  if (configData.SHOW_UV !== undefined) {
    CONFIG.SHOW_UV = configData.SHOW_UV.value;
  }
  if (configData.SHOW_WIND !== undefined) {
    CONFIG.SHOW_WIND = configData.SHOW_WIND.value;
  }
  if (configData.SHOW_TIDE !== undefined) {
    CONFIG.SHOW_TIDE = configData.SHOW_TIDE.value;
  }
  if (configData.SHOW_SUNRISE !== undefined) {
    CONFIG.SHOW_SUNRISE = configData.SHOW_SUNRISE.value;
  }
  if (configData.INVERT !== undefined) {
    CONFIG.INVERT = configData.INVERT.value;
  }
  if (configData.MUNI_ENABLED !== undefined) {
    CONFIG.MUNI_ENABLED = configData.MUNI_ENABLED.value;
  }
  if (configData.MUNI_API_KEY) {
    CONFIG.MUNI_API_KEY = configData.MUNI_API_KEY.value.trim();
  }
  if (configData.MUNI_STOP_CODE) {
    CONFIG.MUNI_STOP_CODE = configData.MUNI_STOP_CODE.value.trim();
  }
  if (configData.MUNI_ROUTE) {
    CONFIG.MUNI_ROUTE = configData.MUNI_ROUTE.value.trim();
  }
  if (configData.MUNI_DIRECTION) {
    CONFIG.MUNI_DIRECTION = configData.MUNI_DIRECTION.value;
  }

  // Pollen configuration
  if (configData.POLLEN_ENABLED !== undefined) {
    CONFIG.POLLEN_ENABLED = configData.POLLEN_ENABLED.value;
  }
  if (configData.POLLEN_API_KEY) {
    CONFIG.POLLEN_API_KEY = configData.POLLEN_API_KEY.value.trim();
  }

  // Save config
  saveConfig();

  // Update weather with new config
  updateWeather();
});
