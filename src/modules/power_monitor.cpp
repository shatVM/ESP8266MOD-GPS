#include "power_monitor.h"

#include <Adafruit_INA219.h>
#include <math.h>

namespace {
constexpr uint8_t INA219_I2C_ADDRESS = 0x40;
constexpr uint32_t UPDATE_INTERVAL_MS = 500;

Adafruit_INA219 ina219(INA219_I2C_ADDRESS);
bool powerMonitorReady = false;
uint32_t lastUpdateMs = 0;
float busVoltage = NAN;
float currentAmpere = NAN;
float powerWatt = NAN;
}

void initPowerMonitor() {
  if (!ina219.begin()) {
    Serial.println("INA219 not found. Check VCC, GND, SDA/D2, SCL/D1 and I2C address.");
    return;
  }

  // Стандартне калібрування INA219 для шунта R100 (0.1 Ом), до 2 A.
  ina219.setCalibration_32V_2A();
  powerMonitorReady = true;
  Serial.println("INA219 ready");
  lastUpdateMs = millis() - UPDATE_INTERVAL_MS;
  updatePowerMonitor();
}

void updatePowerMonitor() {
  if (!powerMonitorReady || millis() - lastUpdateMs < UPDATE_INTERVAL_MS) {
    return;
  }

  lastUpdateMs = millis();
  busVoltage = ina219.getBusVoltage_V();
  currentAmpere = ina219.getCurrent_mA() / 1000.0F;
  // Розрахунок з виміряних напруги та струму лишається стабільним для всіх модулів.
  powerWatt = busVoltage * currentAmpere;
}

float getBusVoltage() { return busVoltage; }
float getCurrent() { return currentAmpere; }
float getPower() { return powerWatt; }

String getPowerSummary() {
  return String(getBusVoltage(), 2) + "V " + String(getCurrent(), 2) + "A";
}
