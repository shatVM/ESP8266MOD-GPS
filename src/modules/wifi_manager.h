#pragma once
#include <Arduino.h>

void initWiFiManager(const char* apSsid = "HelioCore_AI_2", const char* apPassword = "12345678", const char* staSsid = "", const char* staPassword = "");
void updateWiFiManager();
bool isWifiReady();
String getWifiStatusText();
void saveWifiCredentials(const String& ssid, const String& password);
String getAvailableWifiNetworksHtml();
String getWifiModeText();
