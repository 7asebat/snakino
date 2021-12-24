#include <ArduinoQueue.h>
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
  Serial.begin(9600);
  initRandSeed();
  Pin::initPinModes();
  lc.initLedMatrices();
  game.checkAndWaitNewGameStart(&snake, &lc);
}

/************* loop *************/
void loop()
{
  snake.takeAndParseJoystickInput(0);
  snake.takeAndParseJoystickInput(1);

  if (unblockingDelay(&(game.lastGameStep), long(1000.0 / game.gameSpeed)))
    game.stepGame(&snake);

  if (game.gameEndReason)
  {
    game.endGame(&lc);
    game.checkAndWaitNewGameStart(&snake, &lc);
  }

  lc.printGridToMatrix(&(game.grid));
}
