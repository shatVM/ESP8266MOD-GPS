#include "reducer_control.h"

namespace {
int reducerPin = 13;
bool running = false;
bool directionClockwise = true;
}

void initReducerControl(int pin) {
  reducerPin = pin;
  pinMode(reducerPin, OUTPUT);
  digitalWrite(reducerPin, LOW);
}

void startReducer() {
  running = true;
  digitalWrite(reducerPin, HIGH);
}

void stopReducer() {
  running = false;
  digitalWrite(reducerPin, LOW);
}

void setReducerDirection(bool clockwise) {
  directionClockwise = clockwise;
}

bool isReducerRunning() {
  return running;
}
