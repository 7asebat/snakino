#pragma once
#include "Arduino.h"

struct Grid
{
  static const byte cellEmpty = 0;
  static const byte cellFoodLow = 100;
  static const byte cellFoodHigh = 101;
  static const byte cellForceHigh = 200;
  static const byte L = 16;
  static const byte W = 16;
  byte matrix[Grid::L][Grid::W];
};
