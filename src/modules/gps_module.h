#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>

void initGps();
void updateGps();
String getGpsSummary();
TinyGPSPlus& getGpsObject();
