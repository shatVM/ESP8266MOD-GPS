#ifndef TRACKER_LOGIC_H
#define TRACKER_LOGIC_H

#include <Arduino.h>
#include "../config.h"
#include "gps_module.h"
#include "sun_tracker.h"
#include "stepper_control.h"
#include "servo_control.h"

namespace TrackerLogic {
    extern float lastSunAzimuth;
    extern unsigned long nextTrackingTime;
    extern bool isTestMode;
}

inline void initTracker(bool testMode) {
    TrackerLogic::isTestMode = testMode;
    // Встановлюємо час першого руху
    unsigned long firstInterval = TrackerLogic::isTestMode ? TEST_MODE_INTERVAL : WORK_MODE_INTERVAL;
    TrackerLogic::nextTrackingTime = millis() + firstInterval;
}

inline void reInitTracker(bool testMode) {
    // Ця функція дозволяє змінити режим роботи "на льоту"
    TrackerLogic::isTestMode = testMode;
    unsigned long interval = TrackerLogic::isTestMode ? TEST_MODE_INTERVAL : WORK_MODE_INTERVAL;
    TrackerLogic::nextTrackingTime = millis() + interval; // Скидаємо таймер відповідно до нового режиму
}

inline unsigned long getNextTrackingTime() {
    return TrackerLogic::nextTrackingTime;
}

inline void updateTracking() {
    // Перевіряємо, чи настав час для наступного розрахунку
    if (millis() < TrackerLogic::nextTrackingTime) {
        return;
    }

    // Встановлюємо час для наступного руху одразу, щоб уникнути подвійних інтервалів
    unsigned long interval = TrackerLogic::isTestMode ? TEST_MODE_INTERVAL : WORK_MODE_INTERVAL;
    TrackerLogic::nextTrackingTime = millis() + interval;

    if (TrackerLogic::isTestMode) {
        // --- ТЕСТОВИЙ РЕЖИM ---
        // Просто повертаємо на 45 градусів кожні 10 секунд
        float angleToMove = TEST_MODE_FIXED_ANGLE;
        Serial.println("TEST MODE: Moving stepper by " + String(angleToMove) + " degrees.");
        moveStepperByAngle(angleToMove);
    } else {
        // --- РОБОЧИЙ РЕЖИМ ---
        if (USE_SOLAR_POSITION_CALC) {
            // --- РОЗРАХУНОК ЗА SOLAR POSITION ---
            TinyGPSPlus& gpsData = getGpsObject();
            if (!gpsData.location.isValid() || !gpsData.time.isValid() || !gpsData.date.isValid()) {
                Serial.println("Tracking skipped: No GPS data. Retrying in 60s.");
                TrackerLogic::nextTrackingTime = millis() + GPS_RETRY_INTERVAL; 
                return;
            }

            float compassHeading = getCompassHeading(gpsData.location.lat(), gpsData.location.lng());
            float currentSunAzimuth = 0.0f;
            float sunElevation = 0.0;
            int dummyTargetAngle = 0;

            calculateSunTracking(gpsData.location.lat(), gpsData.location.lng(),
                                gpsData.date.year(), gpsData.date.month(), gpsData.date.day(),
                                gpsData.time.hour(), gpsData.time.minute(), gpsData.time.second(),
                                compassHeading, &currentSunAzimuth, &sunElevation, &dummyTargetAngle);

            // Оновлюємо сервопривід висоти
            int elevationAngle = constrain((int)(90.0f + sunElevation), 0, 180);
            setElevationServoAngle(elevationAngle);

            // Розраховуємо кут, на який потрібно повернути трекер
            // Різниця між тим, куди має дивитись трекер (азимут сонця) і куди він дивиться зараз (компас)
            float angleToMove = currentSunAzimuth - compassHeading;

            // Нормалізуємо кут до діапазону [-180, 180]
            if (angleToMove > 180.0f) angleToMove -= 360.0f;
            if (angleToMove < -180.0f) angleToMove += 360.0f;

            Serial.println("WORK MODE: Sun Az: " + String(currentSunAzimuth) + ", Compass: " + String(compassHeading) + ". Moving by: " + String(angleToMove) + " deg.");
            moveStepperByAngle(angleToMove);
        } else {
            // --- РУХ НА ФІКСОВАНИЙ КУТ ---
            float angleToMove = WORK_MODE_FIXED_ANGLE;
            Serial.println("WORK MODE (FIXED): Moving stepper by " + String(angleToMove) + " degrees.");
            moveStepperByAngle(angleToMove);
        }
    }
}

#endif // TRACKER_LOGIC_H