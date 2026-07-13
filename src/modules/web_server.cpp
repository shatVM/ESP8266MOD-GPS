#include "web_server.h"
#include <ESP8266WebServer.h>
#include "../config.h"
#include "servo_control.h"
#include "compass.h"
#include "gps_module.h"
#include "wifi_manager.h"
#include "sun_tracker.h"
#include "tracker_logic.h"
#include "dc_motor_control.h"

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

void handleMotorPulse() {
  if (!isMotorRunning()) {
    moveMotorForDuration(MANUAL_PULSE_DURATION_MS);
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Motor pulsed");
}

}

void handleConfigSave() {
  // Обробка радіокнопок для режиму роботи
  if (server.hasArg("operating_mode")) {
    OPERATING_MODE = server.arg("operating_mode").toInt();
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
  if (server.hasArg("work_angle")) {
    WORK_MODE_FIXED_ANGLE = server.arg("work_angle").toFloat();
  }
  if (server.hasArg("motor_ms_per_degree")) {
    MOTOR_MS_PER_DEGREE = server.arg("motor_ms_per_degree").toInt();
  }
  if (server.hasArg("manual_pulse_duration")) {
    MANUAL_PULSE_DURATION_MS = server.arg("manual_pulse_duration").toInt();
  }

  // Скидаємо таймер, щоб зміни застосувалися негайно
  TrackerLogic::nextTrackingTime = millis();

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
  server.on("/save", HTTP_POST, handleWifiSave); // Додано відсутній обробник
  server.on("/save-config", HTTP_POST, handleConfigSave);
  server.on("/motor-pulse", handleMotorPulse);
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
  html += "<label><input type='radio' name='operating_mode' value='1'" + String(OPERATING_MODE == 1 ? " checked" : "") + "> 1: Test</label><br>";
  html += "<label><input type='radio' name='operating_mode' value='2'" + String(OPERATING_MODE == 2 ? " checked" : "") + "> 2: Backup (Fixed Angle)</label><br>";
  html += "<label><input type='radio' name='operating_mode' value='3'" + String(OPERATING_MODE == 3 ? " checked" : "") + "> 3: Work (GPS)</label><br><br>";
  html += "<label>Test Interval (seconds):</label><br>";
  html += "<input type='number' name='test_interval' value='" + String(TEST_MODE_INTERVAL / 1000) + "'><br>";
  html += "<label>Work Interval (seconds):</label><br>";
  html += "<input type='number' name='work_interval' value='" + String(WORK_MODE_INTERVAL / 1000) + "'><br>";
  html += "<label>Test Mode Angle (degrees):</label><br>";
  html += "<input type='number' name='test_angle' value='" + String(TEST_MODE_FIXED_ANGLE) + "'><br><br>";
  html += "<label>Backup/Work Fixed Angle (degrees):</label><br>";
  html += "<input type='number' step='0.1' name='work_angle' value='" + String(WORK_MODE_FIXED_ANGLE) + "'><br><br>";
  html += "<hr><h4>Motor Calibration & Manual Control</h4>";
  html += "<label>Motor ms per degree:</label><br><input type='number' name='motor_ms_per_degree' value='" + String(MOTOR_MS_PER_DEGREE) + "'><br><br>";
  html += "<label>Manual Pulse Duration (ms):</label><br><input type='number' name='manual_pulse_duration' value='" + String(MANUAL_PULSE_DURATION_MS) + "'><br><br>";
  html += "<button type='submit'>Apply Config</button>";
  html += "</form>";
  html += "<p><a href='/motor-pulse'><button>Pulse Motor</button></a></p>";
  html += "</body></html>";
  return html;
}
