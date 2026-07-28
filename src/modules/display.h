#pragma once
#include <Arduino.h>

void initDisplay();
void updateDisplay(const String& line1, const String& line2, const String& line3);
void updateDisplayBigText(const String& text, uint8_t size);
void turnDisplayOn();
void turnDisplayOff();
