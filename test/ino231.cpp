#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "INA226.h"

// Standard INA226 I2C address when A0 and A1 are connected to GND.
constexpr uint8_t INA226_ADDRESS = 0x40;

// Check the marking on the shunt resistor. "R100" means 0.1 ohm.
constexpr float SHUNT_RESISTANCE_OHM = 0.1F;
// INA226 permits a maximum shunt voltage of about 81.9 mV.
// With a 0.1-ohm shunt, the maximum safe calibrated current is 0.819 A.
constexpr float MAX_CURRENT_A = 0.8F;
constexpr float BATTERY_CAPACITY_MAH = 2000.0F;
constexpr size_t EEPROM_SIZE = 32;
constexpr size_t CONSUMED_MAH_ADDRESS = 0;
constexpr uint32_t SAVE_INTERVAL_MS = 30000;

INA226 ina(INA226_ADDRESS);
float consumedMilliampHours = 0.0F;
uint32_t lastMeasurementMs = 0;
uint32_t lastSaveMs = 0;

void saveConsumedCapacity() {
  EEPROM.put(CONSUMED_MAH_ADDRESS, consumedMilliampHours);
  EEPROM.commit();
  lastSaveMs = millis();
}

void setup() {
  Serial.begin(9600);
  Wire.begin(D2, D1);  // ESP8266: SDA = D2 (GPIO4), SCL = D1 (GPIO5)
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(CONSUMED_MAH_ADDRESS, consumedMilliampHours);
  if (isnan(consumedMilliampHours) || consumedMilliampHours < 0.0F ||
      consumedMilliampHours > BATTERY_CAPACITY_MAH) {
    consumedMilliampHours = 0.0F;
  }

  if (!ina.begin()) {
    Serial.println("INA226 not found. Check VCC, GND, SDA/D2, SCL/D1 and I2C address.");
    while (true) {
      delay(1000);
    }
  }

  const int calibrationResult = ina.setMaxCurrentShunt(MAX_CURRENT_A, SHUNT_RESISTANCE_OHM);
  if (calibrationResult != INA226_ERR_NONE) {
    Serial.print("INA226 calibration error: ");
    Serial.println(calibrationResult);
    while (true) {
      delay(1000);
    }
  }

  // Ensure both bus-voltage and shunt-voltage conversions run continuously.
  ina.setModeShuntBusContinuous();

  Serial.println("INA226 ready");
  Serial.print("Manufacturer ID: 0x");
  Serial.println(ina.getManufacturerID(), HEX);
  Serial.print("Die ID: 0x");
  Serial.println(ina.getDieID(), HEX);
  Serial.println("Send r in Serial Monitor to reset the consumed capacity.");
  lastMeasurementMs = millis();
  lastSaveMs = lastMeasurementMs;
}

void loop() {
  if (Serial.available()) {
    const char command = static_cast<char>(Serial.read());
    if (command == 'r' || command == 'R') {
    consumedMilliampHours = 0.0F;
    saveConsumedCapacity();
    Serial.println("Consumed capacity reset.");
    }
  }

  const uint32_t now = millis();
  const uint32_t elapsedMs = now - lastMeasurementMs;
  lastMeasurementMs = now;
  const float busVoltage = ina.getBusVoltage();
  const float currentMilliamp = ina.getCurrent_mA();
  // Calculate from the two measured quantities. This is more reliable than
  // the INA226 POWER register on modules where that register stays at zero.
  const float powerMilliwatt = busVoltage * currentMilliamp;

  // Count only discharge current. Positive current means VIN+ -> VIN-.
  if (currentMilliamp > 0.0F) {
    consumedMilliampHours += currentMilliamp * elapsedMs / 3600000.0F;
    if (consumedMilliampHours > BATTERY_CAPACITY_MAH) {
      consumedMilliampHours = BATTERY_CAPACITY_MAH;
    }
  }
  if (now - lastSaveMs >= SAVE_INTERVAL_MS) {
    saveConsumedCapacity();
  }

  Serial.print("Bus: ");
  Serial.print(busVoltage, 1);
  Serial.print(" V (raw: 0x");
  Serial.print(ina.getRegister(0x02), HEX);
  Serial.print(")");
  Serial.print(" V\tShunt: ");
  Serial.print(ina.getShuntVoltage_mV(), 1);
  Serial.print(" mV\tCurrent: ");
  Serial.print(currentMilliamp, 1);
  Serial.print(" mA\tPower: ");
  Serial.print(powerMilliwatt, 1);
  Serial.print(" mW\tRemaining: ");
  Serial.print(BATTERY_CAPACITY_MAH - consumedMilliampHours, 1);
  Serial.println(" mAh");

  delay(1000);
}
