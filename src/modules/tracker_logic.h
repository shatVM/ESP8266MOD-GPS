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
}

inline void initTracker() {
    // Встановлюємо час першого руху
    TrackerLogic::nextTrackingTime = millis() + 1000; // Перший рух після старту через 1 сек
}


inline unsigned long getNextTrackingTime() {
    return TrackerLogic::nextTrackingTime;
}

inline void updateTracking() {
    // ВАЖЛИВО: Не плануємо новий рух, поки не завершився попередній
    if (isStepperRunning()) {
        return;
    }

    // Перевіряємо, чи настав час для наступного розрахунку
    if (millis() < TrackerLogic::nextTrackingTime) {
        return;
    }

    // Логіка вибору режиму
    switch (OPERATING_MODE) {
        case 1: // --- РЕЖИМ 1: ТЕСТОВИЙ ---
            TrackerLogic::nextTrackingTime = millis() + TEST_MODE_INTERVAL;
            {
                float angleToMove = TEST_MODE_FIXED_ANGLE;
                Serial.println("MODE 1 (TEST): Moving by " + String(angleToMove) + " degrees.");
                moveStepperByAngle(angleToMove);
            }
            break;

        case 2: // --- РЕЖИМ 2: РЕЗЕРВНИЙ ---
            TrackerLogic::nextTrackingTime = millis() + WORK_MODE_INTERVAL;
            {
                float angleToMove = WORK_MODE_FIXED_ANGLE;
                Serial.println("MODE 2 (BACKUP): Moving by " + String(angleToMove) + " degrees.");
                moveStepperByAngle(angleToMove);
            }
            break;

        case 3: // --- РЕЖИМ 3: РОБОЧИЙ (GPS) ---
            {
                TinyGPSPlus& gpsData = getGpsObject();
                if (!gpsData.location.isValid() || !gpsData.time.isValid() || !gpsData.date.isValid()) {
                    // Якщо немає GPS, виконуємо логіку РЕЖИМУ 2 (Резервний)
                    TrackerLogic::nextTrackingTime = millis() + WORK_MODE_INTERVAL;
                    float angleToMove = WORK_MODE_FIXED_ANGLE;
                    Serial.println("MODE 3 (WORK/NO GPS): Fallback to backup mode. Moving by " + String(angleToMove) + " deg.");
                    moveStepperByAngle(angleToMove);
                } else {
                    // Якщо є GPS, розраховуємо позицію сонця
                    TrackerLogic::nextTrackingTime = millis() + WORK_MODE_INTERVAL;
                    float compassHeading = getCompassHeading(gpsData.location.lat(), gpsData.location.lng());
                    float currentSunAzimuth = 0.0f;
                    float sunElevation = 0.0;
                    int dummyTargetAngle = 0;

                    calculateSunTracking(gpsData.location.lat(), gpsData.location.lng(),
                                        gpsData.date.year(), gpsData.date.month(), gpsData.date.day(),
                                        gpsData.time.hour(), gpsData.time.minute(), gpsData.time.second(),
                                        compassHeading, &currentSunAzimuth, &sunElevation, &dummyTargetAngle);

                    float angleToMove = currentSunAzimuth - compassHeading;

                    if (angleToMove > 180.0f) angleToMove -= 360.0f;
                    if (angleToMove < -180.0f) angleToMove += 360.0f;

                    const float MIN_ANGLE_TO_MOVE = 1.0f;
                    if (abs(angleToMove) > MIN_ANGLE_TO_MOVE) {
                        Serial.println("MODE 3 (WORK/GPS): Sun Az: " + String(currentSunAzimuth) + ", Compass: " + String(compassHeading) + ". Moving by: " + String(angleToMove) + " deg.");
                        moveStepperByAngle(angleToMove);
                    } else {
                        Serial.println("MODE 3 (WORK/GPS): Angle " + String(angleToMove) + " is too small. Waiting.");
                    }
                }
            }
            break;
        default:
            // Якщо обрано неіснуючий режим, нічого не робимо
            TrackerLogic::nextTrackingTime = millis() + 10000; // Повторна перевірка через 10 сек
            break;
    }
}

#endif // TRACKER_LOGIC_H