module.exports = [
  {
    "type": "heading",
    "defaultValue": "FitzFace Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Customize your high-density watchface with weather, environmental data, and tides."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Units & Display"
      },
      {
        "type": "radiogroup",
        "messageKey": "CONFIG_TEMP_UNIT",
        "label": "Temperature Unit",
        "defaultValue": "F",
        "options": [
          {
            "label": "Fahrenheit (°F)",
            "value": "F"
          },
          {
            "label": "Celsius (°C)",
            "value": "C"
          }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "INVERT",
        "label": "Invert Colors",
        "description": "Switch between light and dark theme",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Tide Station"
      },
      {
        "type": "text",
        "defaultValue": "Enter your NOAA tide station ID. Find stations at tidesandcurrents.noaa.gov"
      },
      {
        "type": "input",
        "messageKey": "TIDE_STATION",
        "label": "NOAA Station ID",
        "description": "Default: 9414290 (San Francisco, CA)",
        "defaultValue": "9414290",
        "attributes": {
          "placeholder": "e.g., 9414290",
          "type": "text"
        }
      },
      {
        "type": "text",
        "defaultValue": "Common stations:\n- 9414290 - San Francisco, CA\n- 8518750 - The Battery, NY\n- 9447130 - Seattle, WA\n- 8729108 - Panama City Beach, FL\n- 9410170 - San Diego, CA"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "MUNI Bus Tracking"
      },
      {
        "type": "text",
        "defaultValue": "Display real-time SF MUNI bus arrivals. Get your free API key at 511.org/open-data/token\n\nFind your stop code at 511.org/transit/agencies/stop-id"
      },
      {
        "type": "toggle",
        "messageKey": "MUNI_ENABLED",
        "label": "Show Bus Countdown",
        "defaultValue": false
      },
      {
        "type": "input",
        "messageKey": "MUNI_API_KEY",
        "label": "511.org API Key",
        "description": "Required for bus predictions",
        "defaultValue": "",
        "attributes": {
          "placeholder": "Your API key from 511.org",
          "type": "text"
        }
      },
      {
        "type": "input",
        "messageKey": "MUNI_STOP_CODE",
        "label": "Stop Code",
        "description": "5-digit code from 511.org",
        "defaultValue": "",
        "attributes": {
          "placeholder": "5-digit stop code",
          "type": "text"
        }
      },
      {
        "type": "input",
        "messageKey": "MUNI_ROUTE",
        "label": "Route Number",
        "description": "Bus line to track (e.g., 38R, 44, N)",
        "defaultValue": "",
        "attributes": {
          "placeholder": "e.g., 1, 38R, N",
          "type": "text"
        }
      },
      {
        "type": "radiogroup",
        "messageKey": "MUNI_DIRECTION",
        "label": "Direction",
        "defaultValue": "IB",
        "options": [
          {
            "label": "Inbound (IB)",
            "value": "IB"
          },
          {
            "label": "Outbound (OB)",
            "value": "OB"
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Pollen Tracking"
      },
      {
        "type": "text",
        "defaultValue": "Display pollen levels for tree, grass, and weed. Get your free API key at console.cloud.google.com/apis/library/pollen.googleapis.com"
      },
      {
        "type": "toggle",
        "messageKey": "POLLEN_ENABLED",
        "label": "Show Pollen Count",
        "defaultValue": false
      },
      {
        "type": "input",
        "messageKey": "POLLEN_API_KEY",
        "label": "Google Pollen API Key",
        "description": "Required for pollen data",
        "defaultValue": "",
        "attributes": {
          "placeholder": "Your API key from Google Cloud",
          "type": "text"
        }
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "About"
      },
      {
        "type": "text",
        "defaultValue": "FitzFace v1.0\nDeveloper: Devin Bernosky\n\nA high-density, data-rich watchface for Pebble.\n\nData sources:\n- Weather: Open-Meteo API\n- Air Quality: Open-Meteo\n- Tides: NOAA CO-OPS"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
