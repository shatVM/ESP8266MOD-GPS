#include "compass.h"
#include <Wire.h>

namespace {
constexpr uint8_t QMC5883L_ADDR = 0x0D;
float filteredHeading = 0.0f;
bool compassInitialized = false;
float headingOffset = 0.0f;
bool offsetCalibrated = false;

float calculateMagneticDeclination(float latitudeDeg, float longitudeDeg) {
  if (isnan(latitudeDeg) || isnan(longitudeDeg)) {
    return 0.0f;
  }

  const float latRad = radians(latitudeDeg);
  const float lonRad = radians(longitudeDeg);
  float declination = 0.0f;

  declination += 0.15f * sinf(lonRad * 0.5f) * cosf(latRad);
  declination += 0.10f * sinf(lonRad) * sinf(latRad);
  declination += 0.02f * longitudeDeg * cosf(latRad) / 180.0f;
  declination += 0.01f * latitudeDeg * sinf(lonRad * 0.3f) / 90.0f;

  return constrain(declination, -25.0f, 25.0f);
}
}

void initCompass() {
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x0A);
  Wire.write(0x80);
  Wire.endTransmission();
  delay(50);

  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x0B);
  Wire.write(0x01);
  Wire.endTransmission();
  delay(10);

  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x09);
  Wire.write(0x05);
  Wire.endTransmission();
  delay(50);

  filteredHeading = 0.0f;
  headingOffset = 0.0f;
  offsetCalibrated = false;
  compassInitialized = true;
}

float getCompassHeading(float latitude, float longitude) {
  if (!compassInitialized) {
    return filteredHeading;
  }

  int16_t x = 0, y = 0;
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x09);
  Wire.write(0x1D);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(QMC5883L_ADDR, 6);

  if (Wire.available() >= 6) {
    uint8_t x_lsb = Wire.read();
    uint8_t x_msb = Wire.read();
    uint8_t y_lsb = Wire.read();
    uint8_t y_msb = Wire.read();
    Wire.read();
    Wire.read();
    x = (int16_t)(x_msb << 8 | x_lsb);
    y = (int16_t)(y_msb << 8 | y_lsb);
  }

  if (x == 0 && y == 0) {
    return filteredHeading;
  }

  float heading = atan2(y, x) * RAD_TO_DEG;
  if (heading < 0) heading += 360.0f;

  if (!offsetCalibrated) {
    headingOffset = heading;
    offsetCalibrated = true;
  }

  const float declination = calculateMagneticDeclination(latitude, longitude);
  float correctedHeading = heading - headingOffset + declination;
  if (correctedHeading < 0.0f) correctedHeading += 360.0f;
  if (correctedHeading >= 360.0f) correctedHeading -= 360.0f;

  if (filteredHeading == 0.0f) {
    filteredHeading = correctedHeading;
  } else {
    filteredHeading = filteredHeading * 0.7f + correctedHeading * 0.3f;
    if (filteredHeading < 0.0f) filteredHeading += 360.0f;
    if (filteredHeading >= 360.0f) filteredHeading -= 360.0f;
  }

  return filteredHeading;
}
