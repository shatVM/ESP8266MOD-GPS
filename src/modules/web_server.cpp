#include "web_server.h"
#include <ESP8266WebServer.h>
#include "../config.h"
#include "servo_control.h"
#include "compass.h"
#include "gps_module.h"
#include "wifi_manager.h"
#include "sun_tracker.h"
#include "tracker_logic.h"

namespace {
ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", buildWebPage());
}

String buildSunStatusText() {
  TinyGPSPlus& gpsData = getGpsObject();
  if (!gpsData.location.isValid() || !gpsData.time.isValid() || !gpsData.date.isValid()) {
    return "Sun: no GPS fix";
  }

  float sunAzimuth = 0.0f;
  float sunElevation = 0.0f;
  int targetAngle = 90;
  calculateSunTracking(
    gpsData.location.lat(),
    gpsData.location.lng(),
    gpsData.date.year(),
    gpsData.date.month(),
    gpsData.date.day(),
    gpsData.time.hour(),
    gpsData.time.minute(),
    gpsData.time.second(),
    getCompassHeading(),
    &sunAzimuth,
    &sunElevation,
    &targetAngle
  );

  return "Sun: Az " + String((int)sunAzimuth) + "° / El " + String((int)sunElevation) + "° (target " + String(targetAngle) + "°)";
}

}

void handleConfigSave() {
  // Обробка радіокнопок для режиму роботи
  if (server.hasArg("op_mode")) {
    SERVO_TEST_MODE = server.arg("op_mode") == "test";
  }

  if (server.hasArg("test_interval")) {
    TEST_MODE_INTERVAL = server.arg("test_interval").toInt() * 1000;
  }
  if (server.hasArg("work_interval")) {
    WORK_MODE_INTERVAL = server.arg("work_interval").toInt() * 1000;
  }
  if (server.hasArg("test_angle")) {
    TEST_MODE_FIXED_ANGLE = server.arg("test_angle").toFloat();
  }
  // Чекбокси надсилаються, лише якщо вони позначені
  USE_SOLAR_POSITION_CALC = server.hasArg("use_solar_pos");
  
  if (server.hasArg("work_angle")) {
    WORK_MODE_FIXED_ANGLE = server.arg("work_angle").toFloat();
  }

  // Повторно ініціалізуємо логіку відстеження з новим режимом
  reInitTracker(SERVO_TEST_MODE);

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Config saved for current session");
}

void handleWifiSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  if (ssid.length() > 0) {
    saveWifiCredentials(ssid, password);
    server.send(200, "text/html", "<html><body><h1>Saved</h1><p>Wi-Fi settings saved. Rebooting...</p></body></html>");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "SSID is required");
  }
}

void initWebServer() {
  server.on("/", handleRoot);
  server.on("/save-config", HTTP_POST, handleConfigSave);
  server.begin();
}

void handleWebServer() {
  server.handleClient();
}

String buildWebPage() {
  String html;
  html += "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>ESP8266 Tracker</title></head><body>";
  html += "<h1>ESP8266 Tracker</h1>";
  html += "<p>WiFi: " + getWifiStatusText() + "</p>";
  html += "<p>Mode: " + getWifiModeText() + "</p>";
  html += "<p>Compass: " + String(getCompassHeading(), 1) + "°</p>";
  html += "<p>GPS: " + getGpsSummary() + "</p>";
  html += "<p>Sun: " + buildSunStatusText() + "</p>";
  html += "<h2>Wi-Fi setup</h2>";
  html += "<form action='/save' method='post'>";
  html += "<label>Network:</label><br><select name='ssid'>" + getAvailableWifiNetworksHtml() + "</select><br>";
  html += "<label>Password:</label><br><input type='password' name='password'><br><br>";
  html += "<button type='submit'>Save and reboot</button>";
  html += "</form>";
  html += "<hr><h2>Configuration (current session)</h2>";
  html += "<form action='/save-config' method='post'>";
  html += "<h4>Operating Mode</h4>";
  html += "<label><input type='radio' name='op_mode' value='test'" + String(SERVO_TEST_MODE ? " checked" : "") + "> Test</label> ";
  html += "<label><input type='radio' name='op_mode' value='work'" + String(!SERVO_TEST_MODE ? " checked" : "") + "> Work</label><br><br>";
  html += "<label>Test Interval (seconds):</label><br>";
  html += "<input type='number' name='test_interval' value='" + String(TEST_MODE_INTERVAL / 1000) + "'><br>";
  html += "<label>Work Interval (seconds):</label><br>";
  html += "<input type='number' name='work_interval' value='" + String(WORK_MODE_INTERVAL / 1000) + "'><br>";
  html += "<label>Test Mode Angle (degrees):</label><br>";
  html += "<input type='number' name='test_angle' value='" + String(TEST_MODE_FIXED_ANGLE) + "'><br><br>";
  html += "<hr><h4>Work Mode Settings</h4>";
  html += "<label><input type='checkbox' name='use_solar_pos' value='1'" + String(USE_SOLAR_POSITION_CALC ? " checked" : "") + "> Use Solar Position Calc</label><br><br>";
  html += "<label>Work Mode Fixed Angle (if not using calc):</label><br>";
  html += "<input type='number' step='0.1' name='work_angle' value='" + String(WORK_MODE_FIXED_ANGLE) + "'><br><br>";
  html += "<button type='submit'>Apply Config</button>";
  html += "</form>";
  html += "</body></html>";
  return html;
}
