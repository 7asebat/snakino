
#include <LedControl.h>
#include <ArduinoQueue.h>
/************* *************/

struct point
{
  int i;
  int j;
};

/************* directions *************/
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

/************* led matrix output pins *************/

#define PIN_DIN 8
#define PIN_CS 9
#define PIN_CLK 10

/************* analog stick pins *************/

#define JOYSTICK_1_SW 6
#define JOYSTICK_1_VRX A0
#define JOYSTICK_1_VRY A1

#define JOYSTICK_2_SW 7
#define JOYSTICK_2_VRX A2
#define JOYSTICK_2_VRY A3

/************* analog stick vars *************/

#define JOYSTICK_THRESHOLD 175

/************* game vars *************/
float gameSpeed = 3;
byte gameEnded = 0;
long lastGameStep = 0;

/************* snake vars *************/
#define SNAKES_COUNT 1
byte snkDir[2] = {DIR_RIGHT, DIR_RIGHT};
byte futureSnkDir[2] = {DIR_RIGHT, DIR_RIGHT};
byte snkId[2] = {1, 2};
ArduinoQueue<point> snkPoints[2] = {ArduinoQueue<point>(256), ArduinoQueue<point>(256)};

/************* led matrix variables *************/

#define LED_MATRIX_INTENSITY 0 // 0..15, 0 is lowest
#define LED_MATRIX_COUNT 2

byte ledMatrixIdx[2][2] = {{0, 1}, {2, 3}};

LedControl lc = LedControl(PIN_DIN, PIN_CLK, PIN_CS, LED_MATRIX_COUNT);

/************* game matrix *************/

#define GAME_MATRIX_W 16
#define GAME_MATRIX_L 8

#define GAME_MATRIX_CELL_EMPTY 0
#define GAME_MATRIX_CELL_FOOD 100
#define GAME_MATRIX_CELL_FORCE_HIGH 200

byte gameMatrix[GAME_MATRIX_L][GAME_MATRIX_W];
point foodPos = {-1, -1};

/************* init functions *************/

void initRandSeed()
{
  randomSeed(analogRead(A5));
}

void initPinModes()
{
  pinMode(JOYSTICK_1_SW, INPUT);
  pinMode(JOYSTICK_1_VRX, INPUT);
  pinMode(JOYSTICK_1_VRY, INPUT);
  pinMode(JOYSTICK_2_SW, INPUT);
  pinMode(JOYSTICK_2_VRX, INPUT);
  pinMode(JOYSTICK_2_VRY, INPUT);
}

void initLedMatrices()
{
  for (int i = 0; i < LED_MATRIX_COUNT; i++)
  {
    lc.shutdown(i, false);
    lc.setIntensity(i, LED_MATRIX_INTENSITY);
    lc.clearDisplay(i);
  }
}

void initSnakesOnGrid()
{
  gameMatrix[0][0] = snkId[0];
  gameMatrix[0][1] = snkId[0];
  gameMatrix[0][2] = snkId[0];
  if (SNAKES_COUNT < 2)
    return;
  gameMatrix[15][0] = snkId[1];
  gameMatrix[15][1] = snkId[1];
  gameMatrix[15][2] = snkId[1];
}

void initSnakesInQueue()
{
  snkPoints[0].enqueue(point{0, 0});
  snkPoints[0].enqueue(point{0, 1});
  snkPoints[0].enqueue(point{0, 2});
  if (SNAKES_COUNT < 2)
    return;
  snkPoints[1].enqueue(point{15, 0});
  snkPoints[1].enqueue(point{15, 1});
  snkPoints[1].enqueue(point{15, 2});
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
        lc.setColumn(ledMatrixIdx[ledMatrixIdxRow][ledMatrixIdxCol], 7 - (i % 8), buffer);
        buffer = 0x00;
      }
    }
}

/************* joystick functions *************/
void takeJoystickInput()
{
  int joystickX = analogRead(JOYSTICK_1_VRX);
  int joystickY = analogRead(JOYSTICK_1_VRY);

  byte currentSnakeDir = snkDir[0];

  if (currentSnakeDir != DIR_UP && 1024 - joystickY < JOYSTICK_THRESHOLD)
    setSnakeDir(0, DIR_UP);
  else if (currentSnakeDir != DIR_DOWN && joystickY < JOYSTICK_THRESHOLD)
    setSnakeDir(0, DIR_DOWN);
  else if (currentSnakeDir != DIR_RIGHT && 1024 - joystickX < JOYSTICK_THRESHOLD)
    setSnakeDir(0, DIR_RIGHT);
  else if (currentSnakeDir != DIR_LEFT && joystickX < JOYSTICK_THRESHOLD)
    setSnakeDir(0, DIR_LEFT);
}

/************* util fns *************/
point evalNewPoint(point curPos, byte dir)
{
  if (dir == DIR_UP)
    return point{curPos.i - 1, curPos.j};
  else if (dir == DIR_RIGHT)
    return point{curPos.i, curPos.j + 1};
  else if (dir == DIR_DOWN)
    return point{curPos.i + 1, curPos.j};
  else if (dir == DIR_LEFT)
    return point{curPos.i, curPos.j - 1};
  return curPos;
}

bool unblockingDelay(long *lastMillis, long delay)
{
  if (millis() - *lastMillis < delay)
    return false;

  *lastMillis = millis();

  return true;
}

/************* game fns *************/
bool snakeHeadOnFood(point snakeHead)
{
  return snakeHead.i == foodPos.i && snakeHead.j == foodPos.j;
}

void stepSnakes()
{
  for (int snakeIdx = 0; snakeIdx < SNAKES_COUNT; snakeIdx++)
  {
    point snakeTail = snkPoints[snakeIdx].getHead();
    point snakeHead = snkPoints[snakeIdx].getTail();
    snkDir[snakeIdx] = futureSnkDir[snakeIdx];
    byte snakeDirection = snkDir[snakeIdx];
    byte snakeId = snkId[snakeIdx];

    point newSnakeHead = evalNewPoint(snakeHead, snakeDirection);
    gameMatrix[newSnakeHead.i][newSnakeHead.j] = snakeId;
    snkPoints[snakeIdx].enqueue(newSnakeHead);

    if (!snakeHeadOnFood(newSnakeHead))
    {
      gameMatrix[snakeTail.i][snakeTail.j] = GAME_MATRIX_CELL_EMPTY;
      snkPoints[snakeIdx].dequeue();
    }
  }
}

void setSnakeDir(int snakeIdx, byte direction)
{
  byte currentSnakeDir = snkDir[snakeIdx];
  if (currentSnakeDir == DIR_UP && direction == DIR_DOWN || currentSnakeDir == DIR_DOWN && direction == DIR_UP || currentSnakeDir == DIR_RIGHT && direction == DIR_LEFT || currentSnakeDir == DIR_LEFT && direction == DIR_RIGHT)
    return;

  futureSnkDir[snakeIdx] = direction;
}

void updateFoodPos()
{
  if (gameMatrix[foodPos.i][foodPos.j] != GAME_MATRIX_CELL_FOOD)
    foodPos = point{-1, -1};

  if (foodPos.i != -1 || foodPos.j != -1)
    return;

  byte i = random(GAME_MATRIX_L), j = random(GAME_MATRIX_W);

  if (gameMatrix[i][j] != GAME_MATRIX_CELL_EMPTY)
    return;

  gameMatrix[i][j] = GAME_MATRIX_CELL_FOOD;
  foodPos.i = i, foodPos.j = j;
}

void checkGameEnd()
{
  for (int snakeIdx = 0; snakeIdx < SNAKES_COUNT; snakeIdx++)
  {
    point snakeHead = snkPoints[snakeIdx].getTail();
    if (snakeHead.i >= GAME_MATRIX_L || snakeHead.i < 0 || snakeHead.j < 0 || snakeHead.j >= GAME_MATRIX_W)
      gameEnded = 1;
  }
}

void stepGame()
{
  stepSnakes();
  checkGameEnd();
  updateFoodPos();
}

void endGame()
{
  for (int i = 0; i < GAME_MATRIX_L; i++)
    for (int j = 0; j < GAME_MATRIX_W; j++)
      gameMatrix[i][j] = GAME_MATRIX_CELL_EMPTY;
  printGridToMatrix();
}

/************* dummy fns *************/
void takeKeyboardInput()
{
  while (Serial.available())
  {
    char x = Serial.read();
    if (x == 'W')
      setSnakeDir(0, DIR_UP);
    else if (x == 'S')
      setSnakeDir(0, DIR_DOWN);
    else if (x == 'D')
      setSnakeDir(0, DIR_RIGHT);
    else if (x == 'A')
      setSnakeDir(0, DIR_LEFT);
  }
}

void printGameMatrix()
{
  for (int i = 0; i < GAME_MATRIX_L; i++)
  {
    for (int j = 0; j < GAME_MATRIX_W; j++)
    {
      Serial.print(gameMatrix[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

/************* setup *************/
void setup()
{
  Serial.begin(9600);
  initRandSeed();
  initPinModes();
  initLedMatrices();
  initSnakesInQueue();
  initSnakesOnGrid();
  printGridToMatrix();
}

/************* loop *************/
void loop()
{
  takeJoystickInput();

  if (unblockingDelay(&lastGameStep, long(1000.0 / gameSpeed)))
    stepGame();

  if (gameEnded)
  {
    endGame();
    while (1)
      ;
  }

  printGridToMatrix();
}
