#include "servo_control.h"
#include <Servo.h>

namespace {
Servo azimuthServo;
Servo elevationServo;
bool elevationServoInitialized = false;
int servoPin = 13;
int currentAzimuthAngle = 90;
int targetAzimuthAngle = 90;
int currentElevationAngle = 90;
int targetElevationAngle = 90;
const int step = 10;
const int minAngle = 0;
const int maxAngle = 180;
const int movementStep = 3;
unsigned long lastServoMoveMs = 0;
const unsigned long servoMoveIntervalMs = 40;
const unsigned long servoUpdateIntervalMs = 5UL * 60UL * 1000UL;
const float compassChangeThresholdDeg = 3.0f;
float lastCompassHeading = 0.0f;
bool hasLastCompassHeading = false;
unsigned long lastServoUpdateMs = 0;

void setAngleInternal(Servo& servo, int angle) {
  int clampedAngle = constrain(angle, minAngle, maxAngle);
  servo.write(clampedAngle);
}

}

void initServoControl(int pin) {
  azimuthServo.attach(pin);
  setAngleInternal(azimuthServo, currentAzimuthAngle);
}

void initElevationServoControl(int pin) {
  elevationServo.attach(pin);
  setAngleInternal(elevationServo, currentElevationAngle);
  elevationServoInitialized = true;
}

void setServoAngle(int angle) {
  targetAzimuthAngle = constrain(angle, minAngle, maxAngle);
}

void setElevationServoAngle(int angle) {
  if (!elevationServoInitialized) {
    return;
  }

  targetElevationAngle = constrain(angle, minAngle, maxAngle);
}

void setAzimuthServoAngleFromCompass(float compassHeading, float sunAzimuth) {
  float relativeError = sunAzimuth - compassHeading;
  if (relativeError < -180.0f) relativeError += 360.0f;
  if (relativeError > 180.0f) relativeError -= 360.0f;

  int newTargetAngle = 90 + (int)constrain(relativeError, -45.0f, 45.0f);
  targetAzimuthAngle = constrain(newTargetAngle, minAngle, maxAngle);

  bool shouldUpdate = false;
  if (!hasLastCompassHeading || fabsf(compassHeading - lastCompassHeading) > compassChangeThresholdDeg || millis() - lastServoUpdateMs >= servoUpdateIntervalMs) {
      shouldUpdate = true;
  }

  if (shouldUpdate) {
    lastCompassHeading = compassHeading;
    hasLastCompassHeading = true;
    lastServoUpdateMs = millis();
  }
}

void moveServoLeft() {
  targetAzimuthAngle = constrain(targetAzimuthAngle - step, minAngle, maxAngle);
}

void moveServoRight() {
  targetAzimuthAngle = constrain(targetAzimuthAngle + step, minAngle, maxAngle);
}

int getServoAngle() {
  return currentAzimuthAngle;
}

void moveServosTowardTarget() {
  if (millis() - lastServoMoveMs < servoMoveIntervalMs) {
    return;
  }
  lastServoMoveMs = millis();

  // Азимут
  if (currentAzimuthAngle != targetAzimuthAngle) {
    if (abs(currentAzimuthAngle - targetAzimuthAngle) <= movementStep) {
      currentAzimuthAngle = targetAzimuthAngle;
    } else if (currentAzimuthAngle < targetAzimuthAngle) {
      currentAzimuthAngle += movementStep;
    } else {
      currentAzimuthAngle -= movementStep;
    }
    setAngleInternal(azimuthServo, currentAzimuthAngle);
  }

  // Висота
  if (elevationServoInitialized && currentElevationAngle != targetElevationAngle) {
    if (abs(currentElevationAngle - targetElevationAngle) <= movementStep) {
      currentElevationAngle = targetElevationAngle;
    } else if (currentElevationAngle < targetElevationAngle) {
      currentElevationAngle += movementStep;
    } else {
      currentElevationAngle -= movementStep;
    }
    setAngleInternal(elevationServo, currentElevationAngle);
  }
}
