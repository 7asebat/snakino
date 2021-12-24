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

/************* buzzer pins *************/

#define PIN_BUZZER 2

/************* led matrix output pins *************/

#define PIN_CS_1 8
#define PIN_CS_2 9
#define PIN_CS_3 10
#define PIN_CS_4 11
#define PIN_DIN 12
#define PIN_CLK 13
#define LC_CNT 4

/************* analog stick pins *************/

const int JOYSTICK_1_SW = 6;
#define JOYSTICK_1_VRX A0
#define JOYSTICK_1_VRY A1

#define JOYSTICK_2_SW 7
#define JOYSTICK_2_VRX A2
#define JOYSTICK_2_VRY A3

const byte joystickPin[2][2] = {{JOYSTICK_1_VRX, JOYSTICK_1_VRY}, {JOYSTICK_2_VRX, JOYSTICK_2_VRY}};

/************* analog stick vars *************/

#define JOYSTICK_THRESHOLD 100

/************* game vars *************/
float gameSpeed = 2;
long lastGameStep = 0;

const byte GAME_END_REASON_NONE = 0;
const byte GAME_END_REASON_FIRST_SNAKE_SELF_COLLIDE = 1;
const byte GAME_END_REASON_FIRST_SNAKE_GRID_COLLIDE = 2;
const byte GAME_END_REASON_FIRST_SNAKE_OTHER_COLLIDE = 3;
const byte GAME_END_REASON_SECOND_SNAKE_SELF_COLLIDE = 4;
const byte GAME_END_REASON_SECOND_SNAKE_GRID_COLLIDE = 5;
const byte GAME_END_REASON_SECOND_SNAKE_OTHER_COLLIDE = 6;
const byte GAME_END_TIE = 7;

const byte SNK_GAME_END_REASON_SELF_COLLIDE = 0;
const byte SNK_GAME_END_REASON_GRID_COLLIDE = 1;
const byte SNK_GAME_END_REASON_OTHER_COLLIDE = 2;
const byte snkGameEndReason[2][3] = {{GAME_END_REASON_FIRST_SNAKE_SELF_COLLIDE,
                                      GAME_END_REASON_FIRST_SNAKE_GRID_COLLIDE,
                                      GAME_END_REASON_FIRST_SNAKE_OTHER_COLLIDE},
                                     {GAME_END_REASON_SECOND_SNAKE_SELF_COLLIDE,
                                      GAME_END_REASON_SECOND_SNAKE_GRID_COLLIDE,
                                      GAME_END_REASON_SECOND_SNAKE_OTHER_COLLIDE}};
byte gameEndReason = GAME_END_REASON_NONE;

byte player1Ready = false;
byte player2Ready = false;

/************* snake vars *************/
#define SNAKES_COUNT 2
byte snkDir[2] = {DIR_RIGHT, DIR_RIGHT};
byte futureSnkDir[2] = {DIR_RIGHT, DIR_RIGHT};
point newSnkHead[2] = {point{-1, -1}, point{-1, -1}};
const byte snkId[2] = {1, 2};
ArduinoQueue<point> snkPoints[2] = {ArduinoQueue<point>(256), ArduinoQueue<point>(256)};

/************* led matrix variables *************/

#define LED_MATRIX_INTENSITY 0 // 0..15, 0 is lowest
#define LED_MATRIX_COUNT 4

LedControl lc1 = LedControl(PIN_DIN, PIN_CLK, PIN_CS_1, 1);
LedControl lc2 = LedControl(PIN_DIN, PIN_CLK, PIN_CS_2, 1);
LedControl lc3 = LedControl(PIN_DIN, PIN_CLK, PIN_CS_3, 1);
LedControl lc4 = LedControl(PIN_DIN, PIN_CLK, PIN_CS_4, 1);
LedControl *lcArr[LC_CNT] = {&lc1, &lc2, &lc3, &lc4};
const byte ledMatrixIdx[2][2] = {{0, 2}, {1, 3}};

/************* game matrix *************/

#define GAME_MATRIX_W 16
#define GAME_MATRIX_L 16

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
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(JOYSTICK_1_SW, INPUT_PULLUP);
  pinMode(JOYSTICK_1_VRX, INPUT);
  pinMode(JOYSTICK_1_VRY, INPUT);
  pinMode(JOYSTICK_2_SW, INPUT_PULLUP);
  pinMode(JOYSTICK_2_VRX, INPUT);
  pinMode(JOYSTICK_2_VRY, INPUT);
}

void initLedMatrices()
{
  for (int i = 0; i < LC_CNT; i++)
  {
    LedControl *lc = lcArr[i];
    lc->shutdown(0, false);
    lc->setIntensity(0, LED_MATRIX_INTENSITY);
    lc->clearDisplay(0);
  }
}

void initSnakesOnGrid()
{
  for (int i = 0; i < GAME_MATRIX_L; i++)
    for (int j = 0; j < GAME_MATRIX_W; j++)
      gameMatrix[i][j] = GAME_MATRIX_CELL_EMPTY;

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
  int i = 0, j = 0, ledMatrixIdxRow, ledMatrixIdxCol, colIdx, lcIdx;
  byte buf = 0x00;

  for (i = 0; i < GAME_MATRIX_L; i++)
    for (j = 0; j < GAME_MATRIX_W; j++)
    {
      buf <<= 1;
      buf |= (gameMatrix[i][j] != GAME_MATRIX_CELL_EMPTY) ? 0x01 : 0x00;
      if (j % 8 == 7)
      {
        ledMatrixIdxRow = i / 8;
        ledMatrixIdxCol = j / 8;
        lcIdx = ledMatrixIdx[ledMatrixIdxRow][ledMatrixIdxCol];
        LedControl *lc = lcArr[lcIdx];
        colIdx = (ledMatrixIdxCol % 2) ? 7 - (i % 8) : i % 8;
        buf = (ledMatrixIdxCol % 2) ? reverseb(buf) : buf;
        lc->setRow(0, colIdx, buf);
        buf = 0x00;
      }
    }
}

/************* joystick functions *************/
void takeJoystickInput(int joystickIdx)
{
  int joystickX = analogRead(joystickPin[joystickIdx][0]);
  int joystickY = analogRead(joystickPin[joystickIdx][1]);

  byte currentSnakeDir = snkDir[joystickIdx];

  if (currentSnakeDir != DIR_UP && joystickX < JOYSTICK_THRESHOLD)
    setSnakeDir(joystickIdx, DIR_UP);
  else if (currentSnakeDir != DIR_DOWN && 1024 - joystickX < JOYSTICK_THRESHOLD)
    setSnakeDir(joystickIdx, DIR_DOWN);
  else if (currentSnakeDir != DIR_RIGHT && joystickY < JOYSTICK_THRESHOLD)
    setSnakeDir(joystickIdx, DIR_RIGHT);
  else if (currentSnakeDir != DIR_LEFT && 1024 - joystickY < JOYSTICK_THRESHOLD)
    setSnakeDir(joystickIdx, DIR_LEFT);
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

byte reverseb(byte start)
{
  byte e = 0;
  for (byte i = 0; i < 8; i++)
  {
    e = e << 1;
    e |= ((start >> i) & 0x01);
  }
  return e;
}

bool unblockingDelay(long *lastMillis, long delay)
{
  if (millis() - *lastMillis < delay)
    return false;

  *lastMillis = millis();

  return true;
}

void playBuzzer(int t)
{
  digitalWrite(PIN_BUZZER, HIGH);
  delay(t);
  digitalWrite(PIN_BUZZER, LOW);
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
    point snakeHead = snkPoints[snakeIdx].getTail();
    snkDir[snakeIdx] = futureSnkDir[snakeIdx];
    byte snakeDirection = snkDir[snakeIdx];
    byte snakeId = snkId[snakeIdx];

    point newSnakeHead = evalNewPoint(snakeHead, snakeDirection);
    newSnkHead[snakeIdx] = newSnakeHead;
  }

  checkGameEnd();

  if (gameEndReason)
    return;

  for (int snakeIdx = 0; snakeIdx < SNAKES_COUNT; snakeIdx++)
  {
    point snakeTail = snkPoints[snakeIdx].getHead();
    point newSnakeHead = newSnkHead[snakeIdx];
    byte snakeId = snkId[snakeIdx];
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
    int otherSnakeIdx = 1 - snakeIdx;
    point currentSnakeNewHead = newSnkHead[snakeIdx];
    point otherSnakeNewHead = newSnkHead[otherSnakeIdx];

    if (currentSnakeNewHead.i >= GAME_MATRIX_L || currentSnakeNewHead.i < 0 || currentSnakeNewHead.j < 0 || currentSnakeNewHead.j >= GAME_MATRIX_W)
    {
      if (gameEndReason == snkGameEndReason[otherSnakeIdx][SNK_GAME_END_REASON_OTHER_COLLIDE])
        gameEndReason = GAME_END_TIE;
      else
        gameEndReason = snkGameEndReason[snakeIdx][SNK_GAME_END_REASON_GRID_COLLIDE];
    }

    if (currentSnakeNewHead.i == otherSnakeNewHead.i && currentSnakeNewHead.j == otherSnakeNewHead.j)
      gameEndReason = GAME_END_TIE;

    if (gameMatrix[currentSnakeNewHead.i][currentSnakeNewHead.j] == snkId[snakeIdx])
      gameEndReason = snkGameEndReason[snakeIdx][SNK_GAME_END_REASON_SELF_COLLIDE];

    if (gameMatrix[currentSnakeNewHead.i][currentSnakeNewHead.j] == snkId[otherSnakeIdx])
      gameEndReason = snkGameEndReason[snakeIdx][SNK_GAME_END_REASON_OTHER_COLLIDE];
  }
}

void stepGame()
{
  stepSnakes();
  updateFoodPos();
}

void endGame()
{
  Serial.println("GAME ENDED!");
  if (gameEndReason == GAME_END_REASON_NONE)
    Serial.println("GAME END REASON: NONE");
  else if (gameEndReason == GAME_END_REASON_FIRST_SNAKE_SELF_COLLIDE)
    Serial.println("GAME END REASON: FIRST SNAKE SELF COLLIDE");
  else if (gameEndReason == GAME_END_REASON_FIRST_SNAKE_GRID_COLLIDE)
    Serial.println("GAME END REASON: FIRST SNAKE GRID COLLIDE");
  else if (gameEndReason == GAME_END_REASON_FIRST_SNAKE_OTHER_COLLIDE)
    Serial.println("GAME END REASON: FIRST SNAKE OTHER COLLIDE");
  else if (gameEndReason == GAME_END_REASON_SECOND_SNAKE_SELF_COLLIDE)
    Serial.println("GAME END REASON: SECOND SNAKE SELF COLLIDE");
  else if (gameEndReason == GAME_END_REASON_SECOND_SNAKE_GRID_COLLIDE)
    Serial.println("GAME END REASON: SECOND SNAKE GRID COLLIDE");
  else if (gameEndReason == GAME_END_REASON_SECOND_SNAKE_OTHER_COLLIDE)
    Serial.println("GAME END REASON: SECOND SNAKE OTHER COLLIDE");
  else if (gameEndReason == GAME_END_TIE)
    Serial.println("GAME END REASON: TIE");

  printGridToMatrix();
  playBuzzer(500);
}

void initGameVars()
{
  lastGameStep = millis();
  for(int i=0;i<SNAKES_COUNT;i++)
  {
    snkDir[i] = DIR_RIGHT;
    futureSnkDir[i] = DIR_RIGHT;
    newSnkHead[i] = point{-1, -1};
    snkPoints[i] = ArduinoQueue<point>(256);
  }
  
  foodPos = {-1, -1};
  player1Ready = false;
  player2Ready = false;
  gameEndReason = GAME_END_REASON_NONE;
}

void startGame()
{
  initGameVars();
  initSnakesInQueue();
  initSnakesOnGrid();
  printGridToMatrix();
}

void checkAndWaitNewGameStart()
{
  byte player1SW = 0, player2SW = 0;

  while(!player1Ready || !player2Ready) {
    player1SW = !digitalRead(JOYSTICK_1_SW);
    player2SW = !digitalRead(JOYSTICK_2_SW);
    
    if (!player1Ready && player1SW)
      playBuzzer(100);
  
    if (!player2Ready && player2SW)
      playBuzzer(100);
  
    player1Ready |= player1SW;
    player2Ready |= player2SW;
  }
  startGame();
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
  checkAndWaitNewGameStart();
}

/************* loop *************/
void loop()
{
  takeJoystickInput(0);
  takeJoystickInput(1);

  if (unblockingDelay(&lastGameStep, long(1000.0 / gameSpeed)))
    stepGame();

  if (gameEndReason)
  {
    endGame();
    checkAndWaitNewGameStart();
  }

  printGridToMatrix();
}
