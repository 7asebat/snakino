#pragma once

struct Grid
{
  static const short cellEmpty = 0;
  static const short cellFood = 100;
  static const short cellForceHigh = 200;
  static const short L = 16;
  static const short W = 16;
  short matrix[Grid::L][Grid::W];
};