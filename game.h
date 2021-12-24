#pragma once

#include "grid.h"
#include "constants.h"
#include "snake.h"
#include "led_control.h"

struct Game
{
  Grid grid;

  const float gameSpeed = 2;
  long lastGameStep = 0;
  byte gameEndReason = GameEndReason::none;

  byte player1Ready = false;
  byte player2Ready = false;

  Point foodPos = {-1, -1};

  void initSnakesOnGrid(Snake *snake);

  bool isSnakeHeadOnFood(Point snakeHead);

  void stepSnakes(Snake *snake);

  void updateFoodPos();

  void stepGame(Snake *snake);

  void checkGameEnd(Snake *snake);

  void endGame(LC *lc);

  void initGameVars(Snake *snake);

  void startGame(Snake *snake, LC *lc);

  void checkAndWaitNewGameStart(Snake *snake, LC *lc);
};