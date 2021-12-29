#pragma once

#include "pins.h"
#include "constants.h"

inline static Point evalNewPoint(Point curPos, byte dir)
{
  if (dir == Direction::up)
    return Point{curPos.i - 1, curPos.j};
  else if (dir == Direction::right)
    return Point{curPos.i, curPos.j + 1};
  else if (dir == Direction::down)
    return Point{curPos.i + 1, curPos.j};
  else if (dir == Direction::left)
    return Point{curPos.i, curPos.j - 1};
  return curPos;
}

inline static byte reverseb(byte start)
{
  byte e = 0;
  for (byte i = 0; i < 8; i++)
  {
    e = e << 1;
    e |= ((start >> i) & 0x01);
  }
  return e;
}

inline static bool unblockingDelay(long *lastMillis, long delay)
{
  if (millis() - *lastMillis < delay)
    return false;

  *lastMillis = millis();

  return true;
}

inline static void playBuzzer(int t)
{
  digitalWrite(Pin::buzzer, HIGH);
  delay(t);
  digitalWrite(Pin::buzzer, LOW);
}

inline static void initRandSeed()
{
  randomSeed(analogRead(A5));
}
