#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include <TimeLib.h>
#include <SolarPosition.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "modules/wifi_manager.h"
#include "modules/display.h"
#include "modules/web_server.h"
#include "modules/servo_control.h"
#include "modules/compass.h"
#include "modules/gps_module.h"
#include "modules/sun_tracker.h"
#include "modules/button_handler.h"
#include "modules/stepper_control.h"
#include "modules/tracker_logic.h"

// --- Визначення та ініціалізація глобальних змінних з config.h ---
const char* PROJECT_TITLE = " HC_AI1.8";        // Назва проєкту, що відображається на дисплеї
int OPERATING_MODE = 3;                         // 1: Тестовий, 2: Резервний, 3: Робочий (GPS)
unsigned long TEST_MODE_INTERVAL = 10000;       // Інтервал руху в тестовому режимі (10000 = 10 секунд)
unsigned long WORK_MODE_INTERVAL = 10 * 60000;  // Інтервал руху в робочому режимі (600000 = 10 хвилин)
unsigned long GPS_RETRY_INTERVAL = 60000;       // Інтервал для повторної спроби, якщо немає сигналу GPS (60000 = 1 хвилина)
float TEST_SEQUENCE_ANGLE_1 = 30.0;             // Кут для першого тестового руху
float TEST_SEQUENCE_ANGLE_2 = -60.0;            // Кут для другого тестового руху
float TEST_SEQUENCE_ANGLE_3 = 30.0;             // Кут для третього тестового руху
float TEST_MODE_FIXED_ANGLE = 45.0;             // Фіксований кут повороту в тестовому режимі (після послідовності)
float WORK_MODE_FIXED_ANGLE = 2.5;              // Фіксований кут повороту в робочому режимі (якщо USE_SOLAR_POSITION_CALC = false)
unsigned long STEPPER_STEP_DELAY = 2;           // Затримка між кроками крокового двигуна в мс

// Визначення змінних з tracker_logic.h
float TrackerLogic::lastSunAzimuth = -1.0;
unsigned long TrackerLogic::nextTrackingTime = 0;
// Визначення змінних з button_handler.h
uint8_t ButtonHandler::buttonPin;
ButtonActionCallback ButtonHandler::actionCallback;
int ButtonHandler::lastButtonState = HIGH;
int ButtonHandler::buttonState = HIGH;
unsigned long ButtonHandler::lastDebounceTime = 0;

// Визначення змінних з stepper_control.h
long StepperControl::steps_to_move = 0;
const int StepperControl::step_sequence[8][4] = {
    {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0},
    {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1}
};
int StepperControl::current_step = 0;
unsigned long StepperControl::last_step_time = 0;
// ------------------------------------------------

SolarPosition sunPosition(0.0, 0.0);
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 1000; // Зменшено інтервал оновлення дисплея до 1 секунди
int displayPage = 0;
const int totalDisplayPages = 6; // Збільшено кількість екранів

void updateSystem();
void switchDisplayPage();
int getKyivTimeOffset(int year, int month, int currentDay, int hour);
String getDisplayPageName(int page);

void setup() {
  Serial.begin(9600);
  delay(1000);

  // Спочатку ініціалізуємо шину I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  // Потім ініціалізуємо пристрої на шині (дисплей, компас)
  initDisplay();
  updateDisplay(PROJECT_TITLE, "Initializing...", "");
  initCompass();

  initWiFiManager("HelioCore_AI_1", "12345678"); // Явно передаємо параметри для AP
  initWebServer();
  initStepper();
  initGps();
  initButton(DISPLAY_BUTTON_PIN, switchDisplayPage);
  initTracker(); // Ініціалізуємо логіку відстеження
  
  // Налаштування OTA
  ArduinoOTA.setHostname("esp8266-solar-tracker");
  ArduinoOTA.setPassword("esp8266");
  ArduinoOTA.begin();

  updateDisplay(PROJECT_TITLE, "WAIT FOR GPS FIX", "");
  Serial.println("\n=== РОЗУМНИЙ ТРЕКЕР З OLED ЗАПУЩЕНО ===");

  // Режим 0: Ініціалізація. Завжди виконується при старті.
  Serial.println("!!! MODE 0: Initializing... Starting test sequence.");

  // Створюємо масив з кутами для тестових рухів
  const float test_angles[] = {TEST_SEQUENCE_ANGLE_1, TEST_SEQUENCE_ANGLE_2, TEST_SEQUENCE_ANGLE_3};
  const int num_moves = sizeof(test_angles) / sizeof(test_angles[0]);

  // Виконуємо рухи в циклі
  for (int i = 0; i < num_moves; ++i) {
      String message = "Init Move " + String(i + 1);
      updateDisplay("INITIALIZING", message, "");
      moveStepperByAngle(test_angles[i]);
      // Рух буде оброблятися в головному циклі loop()
      delay(1000); // Просто чекаємо, щоб повідомлення було видно
    }

  Serial.println("!!! MODE 0: Initialization complete.");
  updateDisplay("INIT", "Complete", "");
  delay(1000);
}

void loop() {
  updateWiFiManager();
  handleWebServer();
  updateGps();
  runStepper(); // Обробляємо рух крокового двигуна
  updateTracking(); // Викликаємо оновлену логіку відстеження
  ArduinoOTA.handle();
  handleButton(); // Викликаємо обробник кнопки з нового модуля

  if (millis() - lastDisplayTime >= displayInterval || displayPage == 4) { // Оновлюємо частіше, якщо відкрито екран таймера
    lastDisplayTime = millis();
    updateSystem();
  }
}

int getKyivTimeOffset(int year, int month, int currentDay, int hour) {
    // Україна переходить на літній час (UTC+3) о 03:00 за місцевим часом в останню неділю березня.
    // Це еквівалентно 01:00 UTC.
    // Перехід на зимовий час (UTC+2) відбувається о 04:00 за місцевим часом в останню неділю жовтня.
    // Це еквівалентно 01:00 UTC.
    if (month < 3 || month > 10) return 2;
    if (month > 3 && month < 10) return 3;

    // --- Надійний розрахунок останньої неділі для березня та жовтня ---
    tmElements_t tm;
    tm.Year = year - 1970;
    tm.Day = 1;
    tm.Hour = 0; tm.Minute = 0; tm.Second = 0;
    if (month == 3) { tm.Month = 4; } else { tm.Month = 11; }

    time_t first_of_next_month = makeTime(tm);
    // Віднімаємо одну секунду, щоб отримати останній момент поточного місяця
    time_t last_day_of_month = first_of_next_month - 1;
    // Знаходимо день тижня для останнього дня місяця (1=Нд, ..., 7=Сб)
    int day_of_week = weekday(last_day_of_month);
    // Віднімаємо дні, щоб дістатися до останньої неділі
    time_t last_sunday_time = last_day_of_month - (day_of_week - 1) * SECS_PER_DAY;
    int last_sunday_day = ::day(last_sunday_time); // Явно вказуємо, що викликаємо функцію з TimeLib

    if (month == 3) { // Березень
        if (currentDay > last_sunday_day) return 3;
        if (currentDay < last_sunday_day) return 2;
        // Якщо день збігається, перевіряємо час (перехід о 01:00 UTC)
        return (hour >= 1) ? 3 : 2;
    }
    if (month == 10) { // Жовтень
        if (currentDay > last_sunday_day) return 2;
        if (currentDay < last_sunday_day) return 3;
        // Якщо день збігається, перевіряємо час (перехід о 01:00 UTC)
        return (hour >= 1) ? 2 : 3;
    }
    return 2; // Резервний варіант
}

String getDisplayPageName(int page) {
  switch (page) {
    case 0: return "1: Compass";
    case 1: return "2: Sun";
    case 2: return "3: Actuator";
    case 3: return "4: Stepper";
    case 4: return "5: Timer";
    default: return "6: WiFi";
  }
}

String getOperatingModeName() {
  switch (OPERATING_MODE) {
    case 1: return "1: Test";
    case 2: return "2: Backup";
    case 3: return "3: Work (GPS)";
    default: return "Unknown";
  }
}

String getFormattedTime(TinyGPSPlus& gps) {
    if (!gps.time.isValid() || !gps.date.isValid()) {
        return "N/A";
    }

    // Спочатку отримуємо зміщення, базуючись на часі UTC
    int offset = getKyivTimeOffset(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour());

    // Створюємо час UTC з даних GPS
    tmElements_t tm;
    tm.Year   = gps.date.year() - 1970;
    tm.Month  = gps.date.month();
    tm.Day    = gps.date.day();
    tm.Hour   = gps.time.hour();
    tm.Minute = gps.time.minute();
    tm.Second = gps.time.second();
    time_t utcTime = makeTime(tm);

    // Застосовуємо зміщення для отримання локального часу
    time_t localTime = utcTime + offset * SECS_PER_HOUR;

    char sz[9]; // HH:MM:SS\0
    sprintf(sz, "%02d:%02d:%02d", hour(localTime), minute(localTime), second(localTime));
    return String(sz);
}

void switchDisplayPage() {
  displayPage = (displayPage + 1) % totalDisplayPages;
  lastDisplayTime = millis(); // Скидаємо таймер, щоб екран оновився негайно
  updateSystem();
}

void updateSystem() {
  TinyGPSPlus& gpsData = getGpsObject();
  float trackerHeading = getCompassHeading (
    gpsData.location.isValid() ? gpsData.location.lat() : 0.0f,
    gpsData.location.isValid() ? gpsData.location.lng() : 0.0f
  );
  float sunAzimuth = 0.0f;
  float sunElevation = 0.0f;
  int targetAngle = 90;
  if (gpsData.location.isValid() && gpsData.time.isValid() && gpsData.date.isValid()) {
    calculateSunTracking(
      gpsData.location.lat(),
      gpsData.location.lng(),
      gpsData.date.year(),
      gpsData.date.month(),
      gpsData.date.day(),
      gpsData.time.hour(),
      gpsData.time.minute(),
      gpsData.time.second(),
      trackerHeading,
      &sunAzimuth,
      &sunElevation,
      &targetAngle
    );

  } else {
    // Якщо немає GPS, можна вивести повідомлення
  }

  String line1;
  String line2;
  String line3;


  String wifiInfo = getWifiStatusText();
  if (WiFi.status() == WL_CONNECTED) {
    wifiInfo += " IP:" + WiFi.localIP().toString();
  }

  switch (displayPage) {
    case 0:
      line1 = getDisplayPageName(displayPage) + String(PROJECT_TITLE);
      line2 = "Compass: " + String((int)trackerHeading) + " deg";
      line3 = "GPS SUN tracker";
      break;
    
    case 1:
      line1 = getDisplayPageName(displayPage) + String(PROJECT_TITLE);
      line2 = "Time: " + getFormattedTime(gpsData);
      line3 = "GPS: " + getGpsSummary();
      break;
    case 2:
      line1 = getDisplayPageName(displayPage) + String(PROJECT_TITLE);
      line2 = "Az: " + String((int)sunAzimuth) + " deg";
      line3 = "El: " + String((int)sunElevation) + " deg";
      break;
    case 3:
      line1 = getDisplayPageName(displayPage) + String(PROJECT_TITLE);
      line2 = "Stepper: " + String(isStepperRunning() ? "RUNNING" : "IDLE");
      line3 = "Steps: " + String(getStepsToMove());
      break;
    case 4:
      line1 = getDisplayPageName(displayPage) + String(PROJECT_TITLE);
      {
        long remainingSeconds = (getNextTrackingTime() - millis()) / 1000;
        line2 = "Next move in:" + String(remainingSeconds > 0 ? remainingSeconds : 0) + " sec";
        line3 = "Mode: " + getOperatingModeName();
      }
      break;
    case 5:
      line1 = getDisplayPageName(displayPage) + String(PROJECT_TITLE);
      line2 = "WiFi: " + getWifiStatusText();
      line3 = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "AP: " + WiFi.softAPIP().toString();
      break;
    default:
      line1 = getDisplayPageName(displayPage);
      line2 = WiFi.status() == WL_CONNECTED ? "WiFi OK" : "WiFi AP";
      line3 = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
      break;
  }

  updateDisplay(line1, line2, line3);

  Serial.println("\n====================== МОНІТОР ДАНИХ ======================");
  Serial.print("Компас: "); Serial.print(trackerHeading, 1); Serial.println(" deg");
  Serial.print("GPS: "); Serial.println(getGpsSummary());
  Serial.print("Сонце Az/El: "); Serial.print(sunAzimuth, 1); Serial.print(" / "); Serial.println(sunElevation, 1);
  Serial.print("Цільовий кут: "); Serial.println(targetAngle);
  Serial.println("===========================================================");
}
