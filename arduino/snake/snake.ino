#include <FlipdotPanel.h>
#include <SPI.h>
#include <Wire.h>
#include "Nunchuk.h"
#include "digits.h"

FlipdotPanel flipdots;


// Joystick actions
#define NONE 0
#define UP 1
#define DOWN 2
#define RIGHT 3
#define LEFT 4

byte rowCount = 16;
byte colCount = 30;


// Scoring
int score1 = 0;
int score2 = 0;
int highScore = 0;
int moveCount;

byte playerCount = 2;
byte activePlayer = 1;

bool invert = false;
bool wrapWall = false;

// Positions occupied by snake. Index 0 is the tail, and
// index snakelen-1 is the head.
const byte MAX_SNAKE = 150;
coord snake[MAX_SNAKE];
byte snakelen = 0;

bool foodstate = false;

const byte MAX_FOOD = 5;
coord food[MAX_FOOD];
byte foodlen = 0;


void setFood(bool state) {

  for (byte n=0;  n<foodlen;  n++) {
    flipdots.setPixel(food[n], state, invert);
  }
}

void setSnake(bool state) {

  for (byte n=0;  n<snakelen;  n++) {
    flipdots.setPixel(snake[n], state, invert);
  }
}

void resetGame() {
  score1 = 0;
  score2 = 0;
  activePlayer = 1;
}

void resetHalfRound(byte player) {

    snakelen = 4;
    if (player == 1) {
      snake[0].X = 5;
      snake[0].Y = 8;
      snake[1].X = 6;
      snake[1].Y = 8;
      snake[2].X = 7;
      snake[2].Y = 8;
      snake[3].X = 8;
      snake[3].Y = 8;
    }
    else {
      snake[0].X = 24;
      snake[0].Y = 8;
      snake[1].X = 23;
      snake[1].Y = 8;
      snake[2].X = 22;
      snake[2].Y = 8;
      snake[3].X = 21;
      snake[3].Y = 8;
    }
    foodlen = 0;
    addFoodAwayFromSnakeAndFood(12);
    foodstate = false;

    moveCount = 0;
}



byte joystick() {

    if (nunchuk_read()) {

      int8_t X = nunchuk_joystickX();
      int8_t Y = nunchuk_joystickY();

        if (X < -80  &&  X < Y) {
          return LEFT;
        }
        if (X > 80  &&  X > Y) {
          return RIGHT;
        }
        if (Y < -80  &&  Y < X) {
          return DOWN;
        }
        if (Y > 80  &&  Y > X) {
          return UP;
        }
    }

    return NONE;
}

bool buttonZ() {
  return (nunchuk_buttonZ() != 0);
}

bool buttonC() {
  return (nunchuk_buttonC() != 0);
}


void clearPanel() {
  
  for (byte n=0;  n<4;  n++) {
    flipdots.setAllColumns(0b1010101010101010, 0xffff, invert);
    delay(100);
    flipdots.setAllColumns(0b0101010101010101, 0xffff, invert);
    delay(100);
  }
  
  flipdots.setAllColumns(0x0000, 0xffff, invert);
}

void showSnake() {
  for (byte n=0;  n<snakelen;  n++) {
    flipdots.setPixel(snake[n], true, invert);
  }
}

// Move the head of the snake, optionally getting longer.
// Returns 0 if move is OK, or -1 if snake hits self or walls.
// Returns 1 on food eating.
int8_t moveSnake(byte direction, bool enlongen) {

  coord head = snake[snakelen-1];

  if (enlongen && snakelen == MAX_SNAKE) {
    // Cancel enlongen.
    enlongen = false;
  }

  if (!enlongen) {
    // Turn off tail end
    flipdots.setPixel(snake[0], false, invert);

    // Shift snake
    for (byte n=1;  n<snakelen;  n++) {
      snake[n-1] = snake[n];
    }
  }
  else {
    snakelen++;
  }

  switch(direction){
    case UP:
      if (head.Y == 0)
        return -1;
      head.Y = head.Y-1;
      break;
    case DOWN:
      if (head.Y == rowCount-1)
        return -1;
      head.Y = head.Y+1;
      break;
    case LEFT:
      if (head.X == 0)
        return -1;
      head.X = head.X-1;
      break;
    case RIGHT:
      if (head.X == colCount-1)
        return -1;
      head.X = head.X+1;
      break;
  }

  snake[snakelen-1] = head;
  flipdots.setPixel(head, true, invert);

  if (hitSelf()) {
    // Game over.
    return -1;
  }

  if (hitFood()) {
    return 1;
  }

  return 0;
}

// Detect self-crossing snakeness: is head already in rest of body?
bool hitSelf() {

  coord head = snake[snakelen-1];
  for (byte n=0;  n<snakelen-1;  n++) {
    if (head.X == snake[n].X  &&  head.Y == snake[n].Y)
      return true;
  }

  return false;
}

// Returns true if head is over food. Consumes food.
bool hitFood() {
  coord head = snake[snakelen-1];
  for (byte n=0;  n<foodlen;  n++) {
    if (head.X == food[n].X  &&  head.Y == food[n].Y) {
      // Consume food
      if (n < foodlen-1) {
        food[n] = food[foodlen-1];
      }
      foodlen--;
      return true;
    }
  }

  return false;
}

// Wait until a joystick action, then return it.
byte awaitAction() {
  byte action = NONE;

  while(action != UP && action != DOWN && action != LEFT && action != RIGHT) {
    action = joystick();
  }

  return action;
}

// Add more food.
void moarFood() {

  if (foodlen >= MAX_FOOD)
    return;
  addFoodAwayFromSnakeAndFood(6);
}

// Pick a food location at least dist positions away from snake and other food.
// Try up to 20 times, then give up.
void addFoodAwayFromSnakeAndFood(int8_t dist) {

  bool tooClose = false;

  coord f = {};

  for (byte tries=0;  tries<20;  tries++) {
    f.X = random(colCount);
    f.Y = random(rowCount);

    tooClose = false;

    // Compare with snake
    for (byte n=0;  n<snakelen;  n++) {
      if ((f.X > snake[n].X - dist) && (f.X < snake[n].X + dist) && (f.Y > snake[n].Y - dist) && (f.Y < snake[n].Y + dist)) {
        tooClose = true;
        break;
      }
    }

    if (!tooClose) {
      // Compare with food
      for (byte n=0;  n<foodlen;  n++) {
        if ((f.X > food[n].X - dist) && (f.X < food[n].X + dist) && (f.Y > food[n].Y - dist) && (f.Y < food[n].Y + dist)) {
          tooClose = true;
          break;
        }
      }
    }

    if (!tooClose) {
      food[foodlen++] = f;
      return;
    }
  }

  // No dice.
  return;
}

byte limitRightAngles(byte action, byte prevAction) {
      // Limit direction to right angle moves (no backsies)
    switch (action) {
      case UP:
        if (prevAction == DOWN)
          return prevAction;
        break;
      case DOWN:
        if (prevAction == UP)
          return prevAction;
        break;
      case LEFT:
        if (prevAction == RIGHT)
          return prevAction;
        break;
      case RIGHT:
        if (prevAction == LEFT)
          return prevAction;
    }

  return action;
}

void reverseSnake() {

  delay(1500);

  // Animate reversal
  for (byte n=0;  n<snakelen;  n++) {
    flipdots.setPixel(snake[n], false, invert);
    delay(35);
    flipdots.setPixel(snake[n], true, invert);
    delay(35);
  }

  // Reverse snake
  byte halflen = snakelen >> 1;
  for (byte n=0; n<(halflen);  n++) {
    coord temp = snake[n];
    snake[n] = snake[snakelen-1-n];
    snake[snakelen-1-n] = temp;
  }

  // Animate reversed snake
  for (byte n=0;  n<snakelen;  n++) {
    flipdots.setPixel(snake[n], false, invert);
    delay(35);
    flipdots.setPixel(snake[n], true, invert);
    delay(35);
  }
  
  // Determine forbidden direction
  byte idleAction;

  if (snake[snakelen-1].X - snake[snakelen-2].X > 0) {
    idleAction = LEFT;
  }
  if (snake[snakelen-1].X - snake[snakelen-2].X < 0) {
    idleAction = RIGHT;
  }
  if (snake[snakelen-1].Y - snake[snakelen-2].Y > 0) {
    idleAction = UP;
  }
  if (snake[snakelen-1].Y - snake[snakelen-2].Y < 0) {
    idleAction = DOWN;
  }

  while (awaitAction() == idleAction)
    ;
}

// Play a single round, one or two players.
void playHalfRound(byte player) {

  invert = (player == 2);

  showSnake();

  byte action = NONE;
  byte prevAction = NONE;
  byte halfAction = NONE;
  int speedCount = 0;
  int foodCount = 0;
  int moveDelay = 250;
  
  // Wait for non-forbidden action.
  byte idleAction = LEFT;
  if (player == 2)
    idleAction = RIGHT;

  while (awaitAction() == idleAction)
    ;
  int8_t result = 0;
  bool longen = false;

  while (result > -1) {

    // Snake reversal?
    if (buttonC() && buttonZ()) {
      reverseSnake();
      halfAction = joystick();
      prevAction = halfAction;
    }

    // Handle food
    setFood(foodstate);
    foodstate = !foodstate;

    // Read joystick and move snake
    if (halfAction != NONE) {
      action = halfAction;
    }
    else {
      action = joystick();
    }
  
    if (action == NONE) {
      action = prevAction;
    }

    action = limitRightAngles(action, prevAction);

    prevAction = action;
    result = moveSnake(action, longen);
    longen = false;
    if (result == 1) {
      moarFood();
      longen = true;
    }

    moveCount++;
    speedCount++;
    foodCount++;
    if (speedCount > 120) {
      speedCount = 0;
      if (moveDelay > 50) {
        moveDelay -= 20;
      }
    }
    if (foodCount > 100) {
      foodCount = 0;
      moarFood();
    }

    // Half delay
    delay(moveDelay >> 1);

    // Check joystick between ticks for better gameplay
    halfAction = joystick();

    // Other half of delay.
    delay(moveDelay >> 1);
  }
}

// Play a full game.
void playGame() {
  Serial.println("\n\nPlay snake!");

  for (byte r=1;  r<=3;  r++) {
    Serial.print("Round ");
    Serial.println(r);

    invert = false;
    resetHalfRound(1);
    clearPanel();
    playHalfRound(1);
    setFood(false);
    scoreRound(1);
    delay(5000);
    Serial.print("  Player 1 score: ");
    Serial.println(score1);
    
    if (playerCount == 2) {
      invert = true;
      resetHalfRound(2);
      clearPanel();
      playHalfRound(2);
      setFood(false);
      scoreRound(2);
      delay(5000);
      Serial.print("  Player 2 score: ");
      Serial.println(score2);
    }
  }
}

void displayDigit(byte d, byte row, byte col, uint16_t mask) {
  uint16_t colbits = 0;

  for (byte i=0;  i<4;  i++) {
    colbits = digit_table[d][i] << row;
    flipdots.setColumn(col, colbits, mask, invert);

    col--;
  }
}

void displayScore(int score, byte row, byte col, uint16_t mask) {

  if (score == 0) {
    displayDigit(0, row, col, mask);
  }
  while (score > 0 ) {
    byte n = score % 10;
    displayDigit(n, row, col, mask);

    col -= 5;
    score /= 10;
  }
}

void showCurrentScoreSplit() {

  invert = false;
  flipdots.setAllColumns(0xff00);
  displayScore(score1, 0, 25, 0x00ff);
  invert = true;
  displayScore(score2, 9, 25, 0xff00);
}

void scoreRound(byte player) {

  for (byte n=0;  n<8;  n++) {
    setSnake(false);
    delay(20);
    setSnake(true);
    delay(20);
  }
  delay(1500);

  // Dissolve snake pixel by pixel
  for (byte n=0;  n<snakelen;  n++) {
    flipdots.setPixel(snake[n], false, invert);
    delay(125);
  }

  showCurrentScoreSplit();
  delay(1500);
  byte w = 50;
  int points = snakelen * 25;
  if (player == 1) {
    invert = false;
    int newscore = score1 + points;
    while (score1 < newscore) {
      if (w > 20)
        score1++;
      else {
        score1 += 5;
        if (score1 > newscore)
          score1 = newscore;
      }
      displayScore(score1, 0, 25, 0x00ff);
      delay(w);
      if (w > 4)
        w--;
    }
  }
  else {
    invert = true;
    int newscore = score2 + points;
    while (score2 < newscore) {
      if (w > 20)
        score2++;
      else {
        score2 += 5;
        if (score2 > newscore)
          score2 = newscore;
      }
      displayScore(score2, 9, 25, 0xff00);
      delay(w);
      if (w > 4)
        w--;
    }
  }
}

void showScores() {
  Serial.println("Scores: ");
  Serial.print("  Player 1: ");
  Serial.println(score1);
  Serial.print("  Player 2: ");
  Serial.println(score2);

  if (score1 > highScore) {
    highScore = score1;
  }
  if (score2 > highScore) {
    highScore = score2;
  }

  Serial.print("High score: ");
  Serial.println(highScore);
}

void setup() {

	Serial.begin(115200);
	flipdots.begin();
	SPI.begin();

	Serial.print("No. of panels: ");
	Serial.println(flipdots.panelCount);

	Serial.print("No. of columns: ");
	Serial.println(flipdots.columnCount);

	flipdots.setAllColumns(0x0000);

  Wire.begin();
  nunchuk_init_power(); // A1 and A2 is power supply
  nunchuk_init();
  randomSeed(analogRead(0));
}

void loop() {
  resetGame();
  playGame();
  showScores();
}
