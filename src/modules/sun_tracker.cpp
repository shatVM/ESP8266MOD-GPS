#include "sun_tracker.h"
#include <TimeLib.h>
#include <SolarPosition.h>

bool calculateSunTracking(float latitude, float longitude, int year, int month, int day, int hour, int minute, int second, float compassHeading, float* outAzimuth, float* outElevation, int* outTargetAngle) {
  if (outAzimuth == nullptr || outElevation == nullptr || outTargetAngle == nullptr) {
    return false;
  }

  tmElements_t tm;
  tm.Year = year - 1970;
  tm.Month = month;
  tm.Day = day;
  tm.Hour = hour;
  tm.Minute = minute;
  tm.Second = second;
  time_t utcTime = makeTime(tm);

  SolarPosition currentTrack(latitude, longitude);
  SolarPosition_t sunCoords = currentTrack.getSolarPosition(utcTime);

  *outAzimuth = sunCoords.azimuth;
  *outElevation = sunCoords.elevation;

  float targetAzimuth = sunCoords.azimuth;
  float relativeError = targetAzimuth - compassHeading;
  if (relativeError < -180.0f) relativeError += 360.0f;
  if (relativeError > 180.0f) relativeError -= 360.0f;

  int angle = 90 + (int)constrain(relativeError, -45.0f, 45.0f);
  *outTargetAngle = constrain(angle, 0, 180);
  return true;
}
