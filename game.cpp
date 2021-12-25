#include "game.h"
#include "pins.h"
#include "utils.h"

void Game::initSnakesOnGrid(Snake *snake)
{
  for (int i = 0; i < Grid::L; i++)
    for (int j = 0; j < Grid::W; j++)
      this->grid.matrix[i][j] = Grid::cellEmpty;

  this->grid.matrix[0][0] = snake->snkId[0];
  this->grid.matrix[0][1] = snake->snkId[0];
  this->grid.matrix[0][2] = snake->snkId[0];
  if (Snake::snakesCount < 2)
    return;
  this->grid.matrix[15][0] = snake->snkId[1];
  this->grid.matrix[15][1] = snake->snkId[1];
  this->grid.matrix[15][2] = snake->snkId[1];
}

bool Game::isSnakeHeadOnFood(Point snakeHead)
{
  return snakeHead.i == this->foodPos.i && snakeHead.j == this->foodPos.j;
}

void Game::blinkFood()
{
  Point foodPos = this->foodPos;
  this->grid.matrix[foodPos.i][foodPos.j] = (this->grid.matrix[foodPos.i][foodPos.j] == Grid::cellFoodHigh) ? Grid::cellFoodLow : Grid::cellFoodHigh;
}

void Game::stepSnakes(Snake *snake)
{
  for (int snakeIdx = 0; snakeIdx < Snake::snakesCount; snakeIdx++)
  {
    Point snakeHead = snake->snkPoints[snakeIdx].last();
    snake->snkDir[snakeIdx] = snake->futureSnkDir[snakeIdx];
    byte snakeDirection = snake->snkDir[snakeIdx];
    byte snakeId = snake->snkId[snakeIdx];

    Point newSnakeHead = evalNewPoint(snakeHead, snakeDirection);
    snake->newSnkHead[snakeIdx] = newSnakeHead;
  }

  checkGameEnd(snake);

  if (this->gameEndReason)
    return;

  for (int snakeIdx = 0; snakeIdx < Snake::snakesCount; snakeIdx++)
  {
    Point snakeTail = snake->snkPoints[snakeIdx].first();
    Point newSnakeHead = snake->newSnkHead[snakeIdx];
    byte snakeId = snake->snkId[snakeIdx];
    this->grid.matrix[newSnakeHead.i][newSnakeHead.j] = snakeId;
    snake->snkPoints[snakeIdx].push(newSnakeHead);

    if (!(this->isSnakeHeadOnFood(newSnakeHead)))
    {
      this->grid.matrix[snakeTail.i][snakeTail.j] = Grid::cellEmpty;
      snake->snkPoints[snakeIdx].shift();
    }
  }
}

void Game::updateFoodPos()
{
  if (this->grid.matrix[foodPos.i][foodPos.j] != Grid::cellFoodHigh && this->grid.matrix[foodPos.i][foodPos.j] != Grid::cellFoodLow)
    this->foodPos = Point{-1, -1};

  if (this->foodPos.i != byte(-1) || this->foodPos.j != byte(-1))
    return;

  byte i = random(Grid::L), j = random(Grid::W);

  if (this->grid.matrix[i][j] != Grid::cellEmpty)
    return;

  this->grid.matrix[i][j] = Grid::cellFoodHigh;
  this->foodPos.i = i, this->foodPos.j = j;
}

void Game::stepGame(Snake *snake)
{
  this->stepSnakes(snake);
  this->updateFoodPos();
}

void Game::checkGameEnd(Snake *snake)
{
  for (int snakeIdx = 0; snakeIdx < Snake::snakesCount; snakeIdx++)
  {
    int otherSnakeIdx = 1 - snakeIdx;
    Point currentSnakeNewHead = snake->newSnkHead[snakeIdx];
    Point otherSnakeNewHead = snake->newSnkHead[otherSnakeIdx];

    if (currentSnakeNewHead.i >= Grid::L || currentSnakeNewHead.i < 0 || currentSnakeNewHead.j < 0 || currentSnakeNewHead.j >= Grid::W)
    {
      if (this->gameEndReason == GameEndReasonMap::snakeGameEndReason[otherSnakeIdx][GameEndReasonMap::otherCollide])
        this->gameEndReason = GameEndReason::tie;
      else
        this->gameEndReason = GameEndReasonMap::snakeGameEndReason[snakeIdx][GameEndReasonMap::gridCollide];
    }

    if (currentSnakeNewHead.i == otherSnakeNewHead.i && currentSnakeNewHead.j == otherSnakeNewHead.j)
      this->gameEndReason = GameEndReason::tie;

    if (this->grid.matrix[currentSnakeNewHead.i][currentSnakeNewHead.j] == snake->snkId[snakeIdx])
      this->gameEndReason = GameEndReasonMap::snakeGameEndReason[snakeIdx][GameEndReasonMap::selfCollide];

    if (this->grid.matrix[currentSnakeNewHead.i][currentSnakeNewHead.j] == snake->snkId[otherSnakeIdx])
      this->gameEndReason = GameEndReasonMap::snakeGameEndReason[snakeIdx][GameEndReasonMap::otherCollide];
  }
}

void Game::endGame(LC *lc)
{
  lc->printGridToMatrix(&(this->grid));

  if (this->gameEndReason == GameEndReason::firstSnakeSelfCollide || this->gameEndReason == GameEndReason::firstSnakeGridCollide || this->gameEndReason == GameEndReason::firstSnakeOtherCollide)
  {
    playBuzzer(500);
    delay(500);
    playBuzzer(500);
  }
  else if (this->gameEndReason == GameEndReason::secondSnakeSelfCollide || this->gameEndReason == GameEndReason::secondSnakeGridCollide || this->gameEndReason == GameEndReason::secondSnakeOtherCollide)
  {
    playBuzzer(500);
  }
  else if (this->gameEndReason == GameEndReason::tie)
  {
    playBuzzer(500);
    delay(500);
    playBuzzer(500);
    delay(500);
    playBuzzer(500);
  }
}

void Game::initGameVars(Snake *snake)
{
  this->lastGameStep = millis();
  this->lastFoodBlink = millis();
  this->foodPos = {-1, -1};
  this->player1Ready = false;
  this->player2Ready = false;
  this->gameEndReason = GameEndReason::none;

  snake->initSnakeVars();
}

void Game::startGame(Snake *snake, LC *lc)
{
  this->initGameVars(snake);
  snake->initSnakesInQueue();
  this->initSnakesOnGrid(snake);
  lc->printGridToMatrix(&(this->grid));
}

void Game::checkAndWaitNewGameStart(Snake *snake, LC *lc)
{
  byte player1SW = 0, player2SW = 0;

  while (!(this->player1Ready) || !(this->player2Ready))
  {
    player1SW = !digitalRead(Pin::joystick_1_SW);
    player2SW = !digitalRead(Pin::joystick_2_SW);

    if (!(this->player1Ready) && player1SW)
      playBuzzer(100);

    if (!(this->player2Ready) && player2SW)
      playBuzzer(100);

    this->player1Ready |= player1SW;
    this->player2Ready |= player2SW;
  }

  this->startGame(snake, lc);
}
