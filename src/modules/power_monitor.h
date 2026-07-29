#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <Arduino.h>

// Ініціалізація датчика INA226.
void initPowerMonitor();

// Оновлює кешовані показники; викликати регулярно з loop().
void updatePowerMonitor();

// Значення у вольтах, амперах і ватах відповідно.
float getBusVoltage();
float getCurrent();
float getPower();
String getPowerSummary();

#endif // POWER_MONITOR_H
