#pragma once
#include <Arduino.h>

bool calculateSunTracking(float latitude, float longitude, int year, int month, int day, int hour, int minute, int second, float compassHeading, float* outAzimuth, float* outElevation, int* outTargetAngle);
