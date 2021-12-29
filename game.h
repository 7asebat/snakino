#pragma once

#include "grid.h"
#include "constants.h"
#include "snake.h"
#include "led_control.h"
#include "Arduino.h"

struct Game
{
  Grid grid;
  
  long lastGameStep = 0;

  long lastFoodBlink = 0;
  const short foodBlinkInterval = 200;

  byte gameEndReason = GameEndReason::none;

  byte player1Ready = false;
  byte player2Ready = false;

  Point foodPos = {-1, -1};

  void initSnakesOnGrid(Snake *snake);

  bool isSnakeHeadOnFood(Point snakeHead);

  void blinkFood();

  void stepSnakes(Snake *snake);

  void updateFoodPos();

  void stepGame(Snake *snake);

  void checkGameEnd(Snake *snake);

  void endGame(LC *lc);

  void initGameVars(Snake *snake);

  void startGame(Snake *snake, LC *lc);

  void checkAndWaitNewGameStart(Snake *snake, LC *lc);

  void checkGameReset(Snake *snake, LC *lc);
};