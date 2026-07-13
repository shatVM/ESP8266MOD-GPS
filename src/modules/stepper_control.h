#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>
#include "../config.h"

const int STEPPER_PINS[] = {STEPPER_PIN_IN1, STEPPER_PIN_IN2, STEPPER_PIN_IN3, STEPPER_PIN_IN4};

// Налаштування двигуна 28BYJ-48
const float GEAR_RATIO = 63.68395; // Точне передаточне число
const int STEPS_PER_MOTOR_REVOLUTION = 32; // Кількість кроків на оберт вала двигуна
const int TOTAL_STEPS_PER_OUTPUT_REVOLUTION = STEPS_PER_MOTOR_REVOLUTION * GEAR_RATIO * 2; // У напівкроковому режимі

// Розраховуємо кількість кроків на один градус повороту панелі
const float STEPS_PER_DEGREE = TOTAL_STEPS_PER_OUTPUT_REVOLUTION / 360.0;

namespace StepperControl {
    extern long steps_to_move;
    extern const int step_sequence[8][4];
    extern int current_step;
    extern unsigned long last_step_time;
}

inline void initStepper() {
    for (int pin : STEPPER_PINS) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}

inline void moveStepperByAngle(float angleDeg) {
    StepperControl::steps_to_move += angleDeg * STEPS_PER_DEGREE;
}

inline bool isStepperRunning() {
    return StepperControl::steps_to_move != 0;
}

inline long getStepsToMove() {
    return StepperControl::steps_to_move;
}

inline void runStepper() {
    if (StepperControl::steps_to_move == 0) {
        // Вимикаємо обмотки для економії енергії та зменшення нагріву
        for (int pin : STEPPER_PINS) {
            digitalWrite(pin, LOW);
        }
        return;
    }

    if (millis() - StepperControl::last_step_time > STEPPER_STEP_DELAY) {
        StepperControl::last_step_time = millis();

        if (StepperControl::steps_to_move > 0) {
            StepperControl::current_step++;
            if (StepperControl::current_step > 7) StepperControl::current_step = 0;
            StepperControl::steps_to_move--;
        } else {
            StepperControl::current_step--;
            if (StepperControl::current_step < 0) StepperControl::current_step = 7;
            StepperControl::steps_to_move++;
        }

        for (int i = 0; i < 4; i++) {
            digitalWrite(STEPPER_PINS[i], StepperControl::step_sequence[StepperControl::current_step][i]);
        }
    }
}

#endif // STEPPER_CONTROL_H