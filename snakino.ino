#include <LedControl.h>

// Led Matrix output pins

#define PIN_DIN 8
#define PIN_CS 9
#define PIN_CLK 10

// Led Matrix related constants

#define LED_MATRIX_INTENSITY 0 // 0..15, 0 is lowest
#define LED_MATRIX_COUNT 2

byte ledMatrixIdx[2][2] = {{0, 1}, {2, 3}};

#define LED_MATRIX_BOT_LEFT 0
#define LED_MATRIX_BOT_RIGHT 1
#define LED_MATRIX_TOP_RIGHT 2
#define LED_MATRIX_TOP_LEFT 3

LedControl lc = LedControl(PIN_DIN, PIN_CLK, PIN_CS, LED_MATRIX_COUNT);

// Game Matrix

#define GAME_MATRIX_W 16
#define GAME_MATRIX_L 8

#define GAME_MATRIX_CELL_EMPTY 0
byte gameMatrix[GAME_MATRIX_L][GAME_MATRIX_W];

void initLedMatrices()
{
  for (int i = 0; i < LED_MATRIX_COUNT; i++)
  {
    lc.shutdown(i, false);
    lc.setIntensity(i, LED_MATRIX_INTENSITY);
    lc.clearDisplay(i);
  }
}

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

void setup()
{
  initLedMatrices();
  printGridToMatrix();
}

void loop()
{
}
