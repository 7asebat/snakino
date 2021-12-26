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

void clearEEPROM()
{
  for (int i = 0; i < EEPROM.length(); i++)
    EEPROM.update(i, 0);
}

void fetchDataFromEEPROM()
{
  int x;
  EEPROM.get(0, x);
  if (x != 0xDD)
    return;

  EEPROM.get(10, game);
  EEPROM.get(300, snake);

  game.player1Ready = false;
  game.player2Ready = false;
  game.lastGameStep = 0;
  game.lastFoodBlink = 0;

  Serial.println("Fetched data from EEPROM!");
  Serial.println("game matrix");
  for(int i=0;i<Grid::L;i++)
  {
    for(int j=0;j<Grid::W;j++)
    {
      Serial.print(game.grid.matrix[i][j]); Serial.print("\t");
    }
    Serial.println("");
  }
}

void persistDataToEEPROM()
{
  int x = 0xDD;
  EEPROM.put(0, x);

  EEPROM.put(10, game);
  EEPROM.put(300, snake);
}

/************* setup *************/
void setup()
{
  Serial.begin(9600);
//  clearEEPROM();
  initRandSeed();
  Pin::initPinModes();
  lc.initLedMatrices();
  game.startGame(&snake, &lc);

  fetchDataFromEEPROM();
  lc.printGridToMatrix(&(game.grid));

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
    game.startGame(&snake, &lc);
  }

  lc.printGridToMatrix(&(game.grid));
  persistDataToEEPROM();
}
