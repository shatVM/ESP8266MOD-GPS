#include "power_monitor.h"
#include "INA226.h"
#include <Wire.h> // Додаємо заголовок для роботи з I2C

// Створюємо екземпляр INA226.
// Адреса 0x40 є стандартною, якщо пін A0 та A1 на платі датчика не підключені.
INA226 ina(0x40);

void initPowerMonitor() {
    // Ініціалізуємо датчик. Шина Wire вже ініціалізована в main.cpp.
    if (!ina.begin()) {
        Serial.println("Failed to find INA226 chip");
        return;
    }
    Serial.println("INA226 sensor found!");

    // Встановлюємо значення шунтуючого резистора (0.1 Ом). Це значення має відповідати вашому датчику.
    // Цей метод калібрує датчик для розрахунку струму та потужності.
    // 3.2А - максимальний очікуваний струм, 0.1 - опір шунта в Омах.
    ina.setMaxCurrentShunt(3.2, 0.1);

    // Додаткові налаштування (опціонально, можна розкоментувати для згладжування показників)
    // ina.setAverage(INA226_AVERAGES_16); // Кількість усереднень
    // ina.setBusConversion(1100); // Час перетворення для шини (в мкс)
    // ina.setShuntConversion(1100); // Час перетворення для шунта (в мкс)
}

float getBusVoltage() {
    return ina.getBusVoltage();
}

float getCurrent() {
    // Функція викликається без аргументів, оскільки датчик вже відкалібровано
    return ina.getCurrent(); 
}

float getPower() {
    return ina.getPower();
}

String getPowerSummary() {
    return String(getBusVoltage(), 2) + "V " + String(getCurrent(), 2) + "A";
}