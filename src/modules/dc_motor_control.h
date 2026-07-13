#ifndef DC_MOTOR_CONTROL_H
#define DC_MOTOR_CONTROL_H

#include <Arduino.h>
#include "../config.h"

namespace DCMotorControl {
    extern unsigned long move_until_time;
}

inline void initMotor() {
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    digitalWrite(MOTOR_PWM_PIN, LOW);
}

inline void stopMotor() {
    digitalWrite(MOTOR_PWM_PIN, LOW);
    DCMotorControl::move_until_time = 0;
}

inline bool isMotorRunning() {
    return DCMotorControl::move_until_time > millis();
}

inline void moveMotorByAngle(float angleDeg) {
    if (isMotorRunning()) return; // Не починаємо новий рух, поки не завершився попередній

    // Оскільки XY-MOS не може змінювати напрямок, ми використовуємо абсолютне значення кута
    long duration_ms = abs(angleDeg) * MOTOR_MS_PER_DEGREE;
    if (duration_ms == 0) return;

    DCMotorControl::move_until_time = millis() + duration_ms;
    digitalWrite(MOTOR_PWM_PIN, HIGH); // Просто вмикаємо двигун
    Serial.println("Starting motor for " + String(duration_ms) + "ms.");
}

inline void moveMotorForDuration(long duration_ms) {
    if (isMotorRunning()) return;
    if (duration_ms <= 0) return;

    DCMotorControl::move_until_time = millis() + duration_ms;
    digitalWrite(MOTOR_PWM_PIN, HIGH);
    Serial.println("Starting motor for manual pulse: " + String(duration_ms) + "ms.");
}


inline void runMotor() {
    if (DCMotorControl::move_until_time == 0) {
        return; // Нічого не робимо, якщо немає завдання
    }

    // Перевіряємо, чи не настав час зупинити двигун
    if (millis() >= DCMotorControl::move_until_time) {
        stopMotor();
        Serial.println("Motor stopped.");
    }
}

#endif // DC_MOTOR_CONTROL_H