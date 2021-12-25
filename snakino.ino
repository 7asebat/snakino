#include <EEPROM.h>
#include "pins.h"
#include "utils.h"
#include "constants.h"
#include "led_control.h"
#include "game.h"
#include "snake.h"

/************* *************/

LC lc;
Game game;
Snake snake;

/************* setup *************/
void setup()
{
  initRandSeed();
  Pin::initPinModes();
  lc.initLedMatrices();
  // fetchDataFromEEPROM(&game, &snake);
  game.checkAndWaitNewGameStart(&snake, &lc);
}

/************* loop *************/
void loop()
{
  snake.takeAndParseJoystickInput(0);
  snake.takeAndParseJoystickInput(1);

  if (unblockingDelay(&(game.lastGameStep), long(1000.0 / game.gameSpeed)))
    game.stepGame(&snake);

  if (unblockingDelay(&(game.lastFoodBlink), long(game.foodBlinkInterval)))
    game.blinkFood();

  if (game.gameEndReason)
  {
    game.endGame(&lc);
    game.checkAndWaitNewGameStart(&snake, &lc);
  }

  lc.printGridToMatrix(&(game.grid));
  //  persistDataToEEPROM(&game, &snake);
}
