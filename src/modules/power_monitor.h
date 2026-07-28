#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <Arduino.h>

// Ініціалізація датчика INA226
void initPowerMonitor();

// Функції для отримання даних
float getBusVoltage();
float getCurrent();
float getPower();
String getPowerSummary(); // Функція для отримання зведених даних


#endif // POWER_MONITOR_H