
#include <LedControl.h>
#include <ArduinoQueue.h>
/************* *************/

struct point
{
  int i;
  int j;
};

/************* led matrix output pins *************/

#define PIN_DIN 8
#define PIN_CS 9
#define PIN_CLK 10

/************* led matrix variables *************/

#define LED_MATRIX_INTENSITY 0 // 0..15, 0 is lowest
#define LED_MATRIX_COUNT 2

byte ledMatrixIdx[2][2] = {{0, 2}, {1, 3}};

LedControl lc = LedControl(PIN_DIN, PIN_CLK, PIN_CS, LED_MATRIX_COUNT);

/************* game matrix *************/

#define GAME_MATRIX_W 8
#define GAME_MATRIX_L 16

#define GAME_MATRIX_CELL_EMPTY 0
byte gameMatrix[GAME_MATRIX_L][GAME_MATRIX_W];

/************* led matrix init functions *************/

void initLedMatrices()
{
  for (int i = 0; i < LED_MATRIX_COUNT; i++)
  {
    lc.shutdown(i, false);
    lc.setIntensity(i, LED_MATRIX_INTENSITY);
    lc.clearDisplay(i);
  }
}

/************* led matrix print functions *************/

void printGridToMatrix()
{
  int i = 0, j = 0, ledMatrixIdxRow, ledMatrixIdxCol;
  byte buffer = 0x00;

  for (i = 0; i < GAME_MATRIX_L; i++)
    for (j = 0; j < GAME_MATRIX_W; j++)
    {
      buffer <<= 1;
      buffer |= (gameMatrix[i][j] != GAME_MATRIX_CELL_EMPTY) ? 0x01 : 0x00;
      if (j % 8 == 7)
      {
        ledMatrixIdxRow = i / 8;
        ledMatrixIdxCol = j / 8;
        lc.setRow(ledMatrixIdx[ledMatrixIdxRow][ledMatrixIdxCol], i % 8, buffer);
        buffer = 0x00;
      }
    }
}

/************* directions *************/
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

/************* game vars *************/
byte gameSpeed = 1;

/************* snake vars *************/
#define SNAKES_COUNT 1
byte snkDir[2] = {DIR_RIGHT, DIR_RIGHT};
byte snkId[2] = {1, 2};
ArduinoQueue<point> snkPoints[2] = {ArduinoQueue<point>(256), ArduinoQueue<point>(256)};

/************* snake grid initiation *************/
void initSnakesOnGrid()
{
  for (int snakeIdx = 0; snakeIdx < SNAKES_COUNT; snakeIdx++)
  {
    point snakeHead = snkPoints[snakeIdx].getHead();
    point snakeTail = snkPoints[snakeIdx].getTail();
    gameMatrix[snakeHead.i][snakeHead.j] = snkId[snakeIdx];
    gameMatrix[snakeTail.i][snakeTail.j] = snkId[snakeIdx];
  }
}

void initSnakesInQueue()
{
  snkPoints[0].enqueue(point{0, 0});
  snkPoints[0].enqueue(point{0, 1});
  snkPoints[1].enqueue(point{15, 0});
  snkPoints[1].enqueue(point{15, 1});
}

/************* util fns *************/
point evalNewPoint(point curPos, byte dir)
{
  if (dir == DIR_UP)
    return point { curPos.i - 1, curPos.j };
  else if (dir == DIR_RIGHT)
    return point { curPos.i, curPos.j + 1 };
  else if (dir == DIR_DOWN)
    return point { curPos.i + 1, curPos.j };
  else if (dir == DIR_LEFT)
    return point { curPos.i, curPos.j - 1};
  return curPos;
}

/************* game fns *************/
void stepSnakes()
{
  for (int snakeIdx = 0; snakeIdx < SNAKES_COUNT; snakeIdx++)
  {
    point snakeTail = snkPoints[snakeIdx].getHead();
    point snakeHead = snkPoints[snakeIdx].getTail();
    byte snakeDirection = snkDir[snakeIdx];
    byte snakeId = snkId[snakeIdx];

    point newSnakeHead = evalNewPoint(snakeHead, snakeDirection);
    gameMatrix[newSnakeHead.i][newSnakeHead.j] = snakeId;
    gameMatrix[snakeTail.i][snakeTail.j] = GAME_MATRIX_CELL_EMPTY;

    snkPoints[snakeIdx].enqueue(newSnakeHead);
    snkPoints[snakeIdx].dequeue();
  }
}

void stepGame()
{
  stepSnakes();
//  printGameMatrix();
}

/************* dummy fns *************/
void takeInput()
{
  while (Serial.available())
  {
    char x = Serial.read();
    if (x == 'W')
      snkDir[0] = DIR_UP;
    else if (x == 'S')
      snkDir[0] = DIR_DOWN;
    else if (x == 'D')
      snkDir[0] = DIR_RIGHT;
    else if (x == 'A')
      snkDir[0] = DIR_LEFT;
  }
}

void printGameMatrix()
{
  for(int i=0;i<GAME_MATRIX_L;i++)
  {
    for(int j=0;j<GAME_MATRIX_W;j++)
    {
      Serial.print(gameMatrix[i][j]); Serial.print(" ");
    }
    Serial.println();
  }
}

void setup()
{
  Serial.begin(9600);
  initLedMatrices();
  initSnakesInQueue();
  initSnakesOnGrid();
  printGridToMatrix();
}

void loop()
{
  delay(1000);
  takeInput();
  stepGame();
  printGridToMatrix();
}
