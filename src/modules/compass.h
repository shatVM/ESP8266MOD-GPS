#pragma once
#include <Arduino.h>

void initCompass();
float getCompassHeading(float latitude = 0.0f, float longitude = 0.0f);
