#pragma once
#include "Arduino.h"

/*
* Self defined point struct
*/
struct Point
{
  byte i, j;
};

/*
* Struct containing directions definitions
*/
namespace Direction
{
  static const byte up = 0;
  static const byte right = 1;
  static const byte down = 2;
  static const byte left = 3;
};

namespace Joystick
{
  static const short inputThreshold = 125;
};

namespace GameConsts
{
  const float gameSpeed = 2.5;
};

namespace GameEndReason
{
  static const short none = 0;

  static const short firstSnakeSelfCollide = 1;
  static const short firstSnakeGridCollide = 2;
  static const short firstSnakeOtherCollide = 3;

  static const short secondSnakeSelfCollide = 4;
  static const short secondSnakeGridCollide = 5;
  static const short secondSnakeOtherCollide = 6;

  static const short tie = 7;
};

namespace GameEndReasonMap
{
  static const short selfCollide = 0;
  static const short gridCollide = 1;
  static const short otherCollide = 2;

  static const short snakeGameEndReason[2][3] = {
      {GameEndReason::firstSnakeSelfCollide, GameEndReason::firstSnakeGridCollide, GameEndReason::firstSnakeOtherCollide},
      {GameEndReason::secondSnakeSelfCollide, GameEndReason::secondSnakeGridCollide, GameEndReason::secondSnakeOtherCollide}};
};
