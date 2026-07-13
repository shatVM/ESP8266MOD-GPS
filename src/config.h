#ifndef CONFIG_H
#define CONFIG_H

// --- ЗАГАЛЬНІ НАЛАШТУВАННЯ ---
extern const char* PROJECT_TITLE; // Назва проєкту, що відображається на дисплеї

// --- РЕЖИМИ РОБОТИ ---
// true - тестовий режим, false - робочий режим відстеження сонця
extern bool SERVO_TEST_MODE; // Перемикач режимів: true = тестовий, false = робочий

// --- НАЛАШТУВАННЯ ІНТЕРВАЛІВ РУХУ (в мілісекундах) ---
extern unsigned long TEST_MODE_INTERVAL; // Інтервал руху в тестовому режимі (10000 = 10 секунд)
extern unsigned long WORK_MODE_INTERVAL; // Інтервал руху в робочому режимі (600000 = 10 хвилин)
extern unsigned long GPS_RETRY_INTERVAL; // Інтервал для повторної спроби, якщо немає сигналу GPS

// --- НАЛАШТУВАННЯ ТЕСТОВОГО РЕЖИМУ ---
// Кути для початкової послідовності рухів
extern float TEST_SEQUENCE_ANGLE_1; // Кут для першого тестового руху
extern float TEST_SEQUENCE_ANGLE_2; // Кут для другого тестового руху
extern float TEST_SEQUENCE_ANGLE_3; // Кут для третього тестового руху

// Фіксований кут повороту в тестовому режимі (після початкової посліovaності)
extern float TEST_MODE_FIXED_ANGLE; // Фіксований кут повороту в тестовому режимі

// --- НАЛАШТУВАННЯ РОБОЧОГО РЕЖИМУ ---
// Перемикач режиму розрахунку в робочому режимі
// true - використовувати розрахунок SolarPosition, false - використовувати фіксований кут
extern bool USE_SOLAR_POSITION_CALC;

extern float WORK_MODE_FIXED_ANGLE; // Фіксований кут повороту в робочому режимі, якщо USE_SOLAR_POSITION_CALC = false

// --- НАЛАШТУВАННЯ ПІНІВ (GPIO) ---
// I2C для дисплея та компаса
const int I2C_SDA_PIN = 4; // D2
const int I2C_SCL_PIN = 5; // D1

// Кнопка перемикання екранів
const int DISPLAY_BUTTON_PIN = 12; // D6

// Сервопривід висоти (Elevation)
const int ELEVATION_SERVO_PIN = 14; // D5

// Кроковий двигун (Azimuth) - драйвер ULN2003
const int STEPPER_PIN_IN1 = 16; // D0
const int STEPPER_PIN_IN2 = 0;  // D3
const int STEPPER_PIN_IN3 = 2;  // D4
const int STEPPER_PIN_IN4 = 13; // D7

// Швидкість крокового двигуна (менше = швидше, але може втрачати кроки; більше = повільніше, але надійніше)
extern unsigned long STEPPER_STEP_DELAY; // Затримка між кроками в мілісекундах. Рекомендовано: 1 (швидко) або 2 (надійно).

#endif // CONFIG_H