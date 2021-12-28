#pragma once

#include "LedControl.h"
#include "pins.h"
#include "grid.h"

struct LC
{
  static const short LCCount = 4;
  static const short LCIntensity = 0; // (0..15), 0 is lowest

  LedControl lc1 = LedControl(Pin::LC_DIN, Pin::LC_CLK, Pin::LC_CS_1, 1);
  LedControl lc2 = LedControl(Pin::LC_DIN, Pin::LC_CLK, Pin::LC_CS_2, 1);
  LedControl lc3 = LedControl(Pin::LC_DIN, Pin::LC_CLK, Pin::LC_CS_3, 1);
  LedControl lc4 = LedControl(Pin::LC_DIN, Pin::LC_CLK, Pin::LC_CS_4, 1);
  LedControl *lcArr[LC::LCCount] = {&lc1, &lc2, &lc3, &lc4};
  inline static const byte ledMatrixIdx[2][2] = {{0, 2}, {1, 3}};

  void initLedMatrices();

  void printGridToMatrix(Grid *gameGrid);
};
