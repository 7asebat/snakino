#include "led_control.h"
#include "utils.h"

void LC::initLedMatrices()
{
  for (int i = 0; i < LC::LCCount; i++)
  {
    LedControl *lc = this->lcArr[i];
    lc->shutdown(0, false);
    lc->setIntensity(0, LCConsts::LCIntensity);
    lc->clearDisplay(0);
  }
}

void LC::printGridToMatrix(Grid *gameGrid)
{
  int i = 0, j = 0, ledMatrixIdxRow, ledMatrixIdxCol, rowIdx, lcIdx;
  byte buf = 0x00;

  for (i = 0; i < gameGrid->L; i++)
    for (j = 0; j < gameGrid->W; j++)
    {
      buf <<= 1;
      buf |= (gameGrid->matrix[i][j] != Grid::cellEmpty && gameGrid->matrix[i][j] != Grid::cellFoodLow) ? 0x01 : 0x00;
      if (j % 8 == 7)
      {
        ledMatrixIdxRow = i / 8;
        ledMatrixIdxCol = j / 8;
        lcIdx = LC::ledMatrixIdx[ledMatrixIdxRow][ledMatrixIdxCol];
        LedControl *lc = this->lcArr[lcIdx];
        rowIdx = (ledMatrixIdxCol % 2) ? 7 - (i % 8) : i % 8; // checks if we are in the right column of the led matrices
                                                              // because in real life, the right column is flipped upside down
        buf = (ledMatrixIdxCol % 2) ? reverseb(buf) : buf;
        lc->setRow(0, rowIdx, buf);
        buf = 0x00;
      }
    }
}
