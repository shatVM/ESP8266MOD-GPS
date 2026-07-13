#pragma once
#include <Arduino.h>

void initServoControl(int pin = 13);
void initElevationServoControl(int pin = 14);
void setServoAngle(int angle);
void setAzimuthServoAngleFromCompass(float compassHeading, float sunAzimuth);
void setElevationServoAngle(int angle);
void moveServoLeft();
void moveServoRight();
int getServoAngle();
void moveServosTowardTarget();
