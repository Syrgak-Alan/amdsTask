#pragma once

#include <Arduino.h>

#define ROWS 100
#define COLS 200 // for an augmented matrix

void initializeInverse();
void stepInversion();
void updateLED();

