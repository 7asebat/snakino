#pragma once
#include "Arduino.h"

/*
* Contains all pin definitions
*/
namespace Pin
{
  // Buzzer
  static const short buzzer = 2;

  // Led Control
  static const short LC_CS_1 = 8;
  static const short LC_CS_2 = 9;
  static const short LC_CS_3 = 10;
  static const short LC_CS_4 = 11;
  static const short LC_DIN = 12;
  static const short LC_CLK = 13;

  // Joysticks
  static const short joystick_1_SW = 6;
  static const short joystick_1_VRX = A0;
  static const short joystick_1_VRY = A1;

  static const short joystick_2_SW = 7;
  static const short joystick_2_VRX = A2;
  static const short joystick_2_VRY = A3;

  static void initPinModes()
  {
    pinMode(Pin::buzzer, OUTPUT);
    pinMode(Pin::joystick_1_SW, INPUT_PULLUP);
    pinMode(Pin::joystick_1_VRX, INPUT);
    pinMode(Pin::joystick_1_VRY, INPUT);
    pinMode(Pin::joystick_2_SW, INPUT_PULLUP);
    pinMode(Pin::joystick_2_VRX, INPUT);
    pinMode(Pin::joystick_2_VRY, INPUT);
  }
};

/*
* Contains pin maps for extendible access later.
*/
namespace PinMap
{
  static constexpr short joystick[2][2] = {
      {Pin::joystick_1_VRX, Pin::joystick_1_VRY},
      {Pin::joystick_2_VRX, Pin::joystick_2_VRY}};
};
