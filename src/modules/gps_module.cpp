#include "gps_module.h"
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

namespace {
// Використовуємо D5 (GPIO14) для RX та D8 (GPIO15) для TX
constexpr int GPS_RX_PIN = 14;
constexpr int GPS_TX_PIN = 15;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
TinyGPSPlus gps;
}

void initGps() {
  gpsSerial.begin(9600);
}

void updateGps() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
}

String getGpsSummary() {
  if (!gps.location.isValid()) {
    return "No GPS fix";
  }
  return String(gps.location.lat(), 5) + ", " + String(gps.location.lng(), 5);
}

TinyGPSPlus& getGpsObject() {
  return gps;
}
