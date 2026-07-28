#include "display.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 32;
constexpr int OLED_ADDR = 0x3C;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
}

void initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
    return;
  }
  display.clearDisplay();
  display.display();
}

void updateDisplay(const String& line1, const String& line2, const String& line3) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 12);
  display.println(line2);
  display.setCursor(0, 24);
  display.println(line3);
  display.display();
}

void updateDisplayBigText(const String& text, uint8_t size) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(size);

  // Розраховуємо координати для центрування тексту
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 + 4); // +4 для кращого візуального центрування по висоті
  display.println(text);
  display.display();
}

void turnDisplayOn() {
  display.ssd1306_command(SSD1306_DISPLAYON);
}

void turnDisplayOff() {
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}
