#include "power_monitor.h"

#include "INA226.h"
#include <math.h>

namespace {
constexpr uint8_t INA226_ADDRESS = 0x40;
// Шунт із маркуванням R100 має опір 0.1 Ом. Для INA226 безпечний
// калібрований струм за такого шунта — не більше приблизно 0.819 A.
constexpr float SHUNT_RESISTANCE_OHM = 0.1F;
constexpr float MAX_CURRENT_A = 0.8F;
constexpr uint32_t UPDATE_INTERVAL_MS = 500;

INA226 ina(INA226_ADDRESS);
bool powerMonitorReady = false;
uint32_t lastUpdateMs = 0;
float busVoltage = NAN;
float currentAmpere = NAN;
float powerWatt = NAN;
}

void initPowerMonitor() {
  if (!ina.begin()) {
    Serial.println("INA226 not found. Check VCC, GND, SDA/D2, SCL/D1 and I2C address.");
    return;
  }

  const int calibrationResult = ina.setMaxCurrentShunt(MAX_CURRENT_A, SHUNT_RESISTANCE_OHM);
  if (calibrationResult != INA226_ERR_NONE) {
    Serial.print("INA226 calibration error: ");
    Serial.println(calibrationResult);
    return;
  }

  ina.setModeShuntBusContinuous();
  powerMonitorReady = true;
  Serial.println("INA226 ready");
  lastUpdateMs = millis() - UPDATE_INTERVAL_MS;
  updatePowerMonitor();
}

void updatePowerMonitor() {
  if (!powerMonitorReady || millis() - lastUpdateMs < UPDATE_INTERVAL_MS) {
    return;
  }

  lastUpdateMs = millis();
  busVoltage = ina.getBusVoltage();
  currentAmpere = ina.getCurrent();
  // Не використовуємо регістр POWER: на деяких модулях він повертає нуль.
  powerWatt = busVoltage * currentAmpere;
}

float getBusVoltage() { return busVoltage; }
float getCurrent() { return currentAmpere; }
float getPower() { return powerWatt; }

String getPowerSummary() {
  return String(getBusVoltage(), 2) + "V " + String(getCurrent(), 2) + "A";
}
