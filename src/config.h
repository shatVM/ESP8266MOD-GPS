#ifndef CONFIG_H
#define CONFIG_H

// --- ЗАГАЛЬНІ НАЛАШТУВАННЯ ---
extern const char* PROJECT_TITLE; // Назва проєкту, що відображається на дисплеї

// --- РЕЖИМИ РОБОТИ ---
// 1: Тестовий, 2: Резервний (фіксований кут), 3: Робочий (GPS)
extern int OPERATING_MODE;

// --- НАЛАШТУВАННЯ ДИСПЛЕЯ ---
extern unsigned long DISPLAY_TIMEOUT_MS; // Час в мс, через який дисплей вимкнеться

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

// --- НАЛАШТУВАННЯ РУЧНОГО КЕРУВАННЯ ---
extern unsigned long MANUAL_PULSE_DURATION_MS; // Тривалість імпульсу для ручного керування

// --- НАЛАШТУВАННЯ РОБОЧОГО РЕЖИМУ ---
extern float WORK_MODE_FIXED_ANGLE; // Фіксований кут повороту в робочому режимі, якщо USE_SOLAR_POSITION_CALC = false

// --- НАЛАШТУВАННЯ ПІНІВ (GPIO) ---
// I2C для дисплея та компаса
const int I2C_SDA_PIN = 4; // GPIO4, D2
const int I2C_SCL_PIN = 5; // GPIO5, D1

// Кнопка перемикання екранів
const int DISPLAY_BUTTON_PIN = 2; // GPIO2, D4 (пін з вбудованим синім світлодіодом)

// Сервопривід висоти (Elevation)
const int ELEVATION_SERVO_PIN = 14; // GPIO14, D5 (не використовується)

// DC-двигун з редуктором (Azimuth) - драйвер XY-MOS
const int MOTOR_PWM_PIN = 12; // GPIO12, D6 (перенесено на безпечний пін)

// Калібрування двигуна: скільки мілісекунд потрібно працювати двигуну для повороту на 1 градус
extern int MOTOR_MS_PER_DEGREE; // !!! ПОТРІБНО ПІДІБРАТИ ЕКСПЕРИМЕНТАЛЬНО !!!

#endif // CONFIG_H