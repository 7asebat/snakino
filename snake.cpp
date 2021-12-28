#include "snake.h"

void Snake::initSnakeVars()
{
  for (int i = 0; i < Snake::snakesCount; i++)
  {
    this->snkDir[i] = Direction::right;
    this->futureSnkDir[i] = Direction::right;
    this->newSnkHead[i] = Point{-1, -1};
    this->snkPoints[i].clear();
  }
}

void Snake::initSnakesInQueue()
{
  int k = (Grid::L / (Snake::snakesCount + 1));
  for (int i = 0; i < Snake::snakesCount; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      this->snkPoints[i].push(Point{k * (i + 1), j});
    }
  }
}

void Snake::setSnakeDir(int snakeIdx, byte direction)
{
  byte currentSnakeDir = this->snkDir[snakeIdx];
  if (currentSnakeDir == Direction::up && direction == Direction::down || currentSnakeDir == Direction::down && direction == Direction::up || currentSnakeDir == Direction::right && direction == Direction::left || currentSnakeDir == Direction::left && direction == Direction::right)
    return;

  this->futureSnkDir[snakeIdx] = direction;
}

void Snake::takeAndParseJoystickInput(int joystickIdx)
{
  int joystickX = analogRead(PinMap::joystick[joystickIdx][0]);
  int joystickY = analogRead(PinMap::joystick[joystickIdx][1]);

  byte currentSnakeDir = this->snkDir[joystickIdx];

  if (currentSnakeDir != Direction::up && joystickX < Joystick::inputThreshold)
    setSnakeDir(joystickIdx, Direction::up);
  else if (currentSnakeDir != Direction::down && 1024 - joystickX < Joystick::inputThreshold)
    setSnakeDir(joystickIdx, Direction::down);
  else if (currentSnakeDir != Direction::right && joystickY < Joystick::inputThreshold)
    setSnakeDir(joystickIdx, Direction::right);
  else if (currentSnakeDir != Direction::left && 1024 - joystickY < Joystick::inputThreshold)
    setSnakeDir(joystickIdx, Direction::left);
}
