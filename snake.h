#pragma once

#include "queue.h"
#include "constants.h"
#include "pins.h"

struct Snake
{
  static const short snakesCount = 2;

  byte snkDir[2] = {Direction::right, Direction::right};
  byte futureSnkDir[2] = {Direction::right, Direction::right};
  Point newSnkHead[2] = {Point{-1, -1}, Point{-1, -1}};
  const byte snkId[2] = {1, 2};
  Queue<Point, 200> snkPoints[2] = {Queue<Point, 200>{}, Queue<Point, 200>{}};

  void initSnakeVars();
  void initSnakesInQueue();
  void setSnakeDir(int snakeIdx, byte direction);
  void takeAndParseJoystickInput(int joystickIdx);
};
