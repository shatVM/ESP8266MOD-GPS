#include "wifi_manager.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

namespace {
constexpr int EEPROM_SIZE = 512;
constexpr int SSID_ADDR = 0;
constexpr int PASS_ADDR = 64;
String apSsid = "HelioCore_AI_1";
String apPassword = "12345678";
String staSsid;
String staPassword;
bool initialized = false;
bool otaInitialized = false;
unsigned long lastStatusLog = 0;
wl_status_t lastWiFiStatus = WL_IDLE_STATUS;

void writeStringToEeprom(int addr, const String& value) {
  for (int i = 0; i < value.length(); ++i) {
    EEPROM.write(addr + i, value[i]);
  }
  EEPROM.write(addr + value.length(), 0);
}

String readStringFromEeprom(int addr) {
  String value;
  char c;
  int i = 0;
  do {
    c = static_cast<char>(EEPROM.read(addr + i));
    if (c != 0) {
      value += c;
    }
    ++i;
  } while (c != 0 && i < 64);
  return value;
}

void startOtaIfNeeded() {
  if (otaInitialized || WiFi.status() != WL_CONNECTED) {
    return;
  }

  ArduinoOTA.setHostname("esp8266-tracker");
  ArduinoOTA.setPassword("esp8266");
  ArduinoOTA.begin();
  otaInitialized = true;
  Serial.println("OTA ready");
}
}

void initWiFiManager(const char* apSsidArg, const char* apPasswordArg, const char* staSsidArg, const char* staPasswordArg) {
  apSsid = apSsidArg ? apSsidArg : "HelioCore_AI_1";
  apPassword = apPasswordArg ? apPasswordArg : "12345678";
  staSsid = staSsidArg ? staSsidArg : "";
  staPassword = staPasswordArg ? staPasswordArg : "";

  EEPROM.begin(EEPROM_SIZE);
  if (staSsid.length() == 0) {
    staSsid = readStringFromEeprom(SSID_ADDR);
    staPassword = readStringFromEeprom(PASS_ADDR);
  }
  EEPROM.end();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSsid.c_str(), apPassword.c_str());

  if (staSsid.length() > 0) {
    Serial.println("Trying to connect to saved WiFi: " + staSsid);
    WiFi.begin(staSsid.c_str(), staPassword.c_str());
  } else {
    Serial.println("No saved WiFi credentials. Starting AP setup mode.");
  }

  initialized = true;
}

void updateWiFiManager() {
  if (!initialized) {
    return;
  }

  if (WiFi.status() != lastWiFiStatus && millis() - lastStatusLog > 1000) {
    lastStatusLog = millis();
    lastWiFiStatus = WiFi.status();
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected to WiFi. IP: ");
      Serial.println(WiFi.localIP());
    } else if (WiFi.softAPgetStationNum() > 0) {
      Serial.print("AP clients: ");
      Serial.println(WiFi.softAPgetStationNum());
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    startOtaIfNeeded();
    ArduinoOTA.handle();
  }
}

bool isWifiReady() {
  return initialized && (WiFi.status() == WL_CONNECTED || WiFi.softAPgetStationNum() > 0);
}

String getWifiStatusText() {
  if (!initialized) {
    return "WiFi not initialized";
  }

  if (WiFi.status() == WL_CONNECTED) {
    return "STA: " + WiFi.localIP().toString();
  }

  return "AP: " + WiFi.softAPIP().toString();
}

void saveWifiCredentials(const String& ssid, const String& password) {
  EEPROM.begin(EEPROM_SIZE);
  writeStringToEeprom(SSID_ADDR, ssid);
  writeStringToEeprom(PASS_ADDR, password);
  EEPROM.commit();
  EEPROM.end();
  staSsid = ssid;
  staPassword = password;
  Serial.println("Saved WiFi credentials. Rebooting...");
}

String getAvailableWifiNetworksHtml() {
  int n = WiFi.scanNetworks(false, true);
  String html;
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    html += "<option value='" + ssid + "'>" + ssid + "</option>";
  }
  return html;
}

String getWifiModeText() {
  if (WiFi.status() == WL_CONNECTED) {
    return "Connected to WiFi";
  }
  if (WiFi.softAPgetStationNum() > 0) {
    return "AP active with clients";
  }
  return "AP setup mode";
}
