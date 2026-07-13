#pragma once
#include <Arduino.h>

void initReducerControl(int pin = 13);
void startReducer();
void stopReducer();
void setReducerDirection(bool clockwise);
bool isReducerRunning();
