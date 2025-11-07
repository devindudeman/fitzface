module.exports = [
  {
    "type": "heading",
    "defaultValue": "FitzFace Configuration",
    "size": 1
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Display",
        "size": 3
      },
      {
        "type": "radiogroup",
        "messageKey": "CONFIG_TEMP_UNIT",
        "label": "Temperature Unit",
        "defaultValue": "F",
        "options": [
          {
            "label": "Fahrenheit",
            "value": "F"
          },
          {
            "label": "Celsius",
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
        "defaultValue": "Tide Station",
        "size": 3
      },
      {
        "type": "input",
        "messageKey": "TIDE_STATION",
        "label": "NOAA Station ID",
        "description": "Find your station at tidesandcurrents.noaa.gov<br><small>San Francisco: 9414290 | New York: 8518750 | Seattle: 9447130</small>",
        "defaultValue": "9414290",
        "attributes": {
          "placeholder": "e.g., 9414290",
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
        "defaultValue": "SF MUNI Bus Tracking",
        "size": 3
      },
      {
        "type": "toggle",
        "messageKey": "MUNI_ENABLED",
        "label": "Enable Bus Countdown",
        "description": "Show real-time SF MUNI arrival times",
        "defaultValue": false
      },
      {
        "type": "input",
        "messageKey": "MUNI_API_KEY",
        "label": "511.org API Key",
        "description": "Get your free key at <strong>511.org/open-data/token</strong>",
        "defaultValue": "",
        "attributes": {
          "placeholder": "API key",
          "type": "text"
        }
      },
      {
        "type": "input",
        "messageKey": "MUNI_STOP_CODE",
        "label": "Stop Code",
        "description": "Find your 5-digit code at <strong>511.org/transit/agencies/stop-id</strong>",
        "defaultValue": "",
        "attributes": {
          "placeholder": "5-digit code",
          "type": "text"
        }
      },
      {
        "type": "input",
        "messageKey": "MUNI_ROUTE",
        "label": "Route Number",
        "description": "Bus line to track (e.g., 1, 38R, N)",
        "defaultValue": "",
        "attributes": {
          "placeholder": "e.g., 38R",
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
            "label": "Inbound",
            "value": "IB"
          },
          {
            "label": "Outbound",
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
        "defaultValue": "Pollen Tracking",
        "size": 3
      },
      {
        "type": "toggle",
        "messageKey": "POLLEN_ENABLED",
        "label": "Enable Pollen Data",
        "description": "Show tree, grass, and weed pollen levels",
        "defaultValue": false
      },
      {
        "type": "input",
        "messageKey": "POLLEN_API_KEY",
        "label": "Google Pollen API Key",
        "description": "Get your free key at <strong>console.cloud.google.com</strong><br><small>Enable the Pollen API in Google Cloud Console</small>",
        "defaultValue": "",
        "attributes": {
          "placeholder": "API key",
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
        "defaultValue": "About FitzFace",
        "size": 3
      },
      {
        "type": "text",
        "defaultValue": "<strong>Version 1.0.0</strong><br>Developer: Devin Bernosky<br>License: GPL-3.0"
      },
      {
        "type": "text",
        "defaultValue": "High-density watchface featuring weather, MUNI bus tracking, tides, air quality, pollen levels, and smart health alerts."
      },
      {
        "type": "heading",
        "defaultValue": "Data Sources",
        "size": 5
      },
      {
        "type": "text",
        "defaultValue": "<small>Weather: Open-Meteo | AQI: Open-Meteo | Tides: NOAA<br>MUNI: 511.org | Pollen: Google | Location: OpenStreetMap</small>"
      },
      {
        "type": "heading",
        "defaultValue": "Links",
        "size": 5
      },
      {
        "type": "text",
        "defaultValue": "<a href='https://github.com/devindudeman/fitzface' target='_blank'>GitHub Repository</a>"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
