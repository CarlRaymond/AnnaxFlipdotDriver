#include <SPI.h>
#include <Wire.h>
#include "Nunchuk.h"

#define COL_REG_CLK 4
#define ROW_REG_CLK 5
#define COL_EN 6
#define ROW_EN 7
#define SENSE_EN 8
#define RC_CLEAR 9
#define RC_DATA 11
#define RC_SHIFT_CLK 13
#define SENSE 12

#define CTRL_A0 3
#define CTRL_A1 14
#define CTRL_A2 16
#define CTRL_A3 18

#define CTRL_B0 2
#define CTRL_B1 15
#define CTRL_B2 17
#define CTRL_B3 19


// Joystick moves
#define NONE 0
#define UP 1
#define DOWN 2
#define RIGHT 3
#define LEFT 4
#define BUTTON 5

const byte rowCount = 16;
const byte colCount = 30;

SPISettings SPImode = SPISettings(1000000, LSBFIRST, SPI_MODE0);

// Delay time between snake moves
int moveDelay = 100;

// Scoring
int score = 0;
int highScore = 0;

bool wrapWall = false;

struct coord {
    byte X;
    byte Y;
};

// Positions occupied by snake. Index 0 is the tail, and
// index snakelen-1 is the head.
const byte MAX_SNAKE = 150;
coord snake[MAX_SNAKE];
byte snakelen = 0;

void resetGame() {
    snakelen = 4;
    snake[0].X = 5;
    snake[0].Y = 8;
    snake[1].X = 6;
    snake[1].Y = 8;
    snake[2].X = 7;
    snake[2].Y = 8;
    snake[3].X = 8;
    snake[3].Y = 8;
    score = 0;
}

//
// Bit pairs for various outputs:
//
// HL
// 00: HIGH: high side on (pulled high), low side off
// 01: XLOW: diode prevents pullup enabling high side, low side on
// 10: OFF: high side off, low side off
// 11: LOW: high side off, low side on

byte colVec[8];

void setupPanel() {
  pinMode(COL_REG_CLK, OUTPUT);
  pinMode(ROW_REG_CLK, OUTPUT);
  pinMode(COL_EN, OUTPUT);
  pinMode(ROW_EN, OUTPUT);
  pinMode(RC_CLEAR, OUTPUT);
  pinMode(RC_DATA, OUTPUT);
  pinMode(RC_SHIFT_CLK, OUTPUT);
  pinMode(SENSE_EN, OUTPUT);

  pinMode(CTRL_A0, INPUT);
  pinMode(CTRL_A1, INPUT);
  pinMode(CTRL_A2, INPUT);
  pinMode(CTRL_A3, INPUT);
  pinMode(CTRL_B0, INPUT);
  pinMode(CTRL_B1, INPUT);
  pinMode(CTRL_B2, INPUT);
  pinMode(CTRL_B3, INPUT);
  pinMode(SENSE, INPUT);

  digitalWrite(SENSE_EN, LOW);

  // Disable outputs -- turns on all high sides on, low sides off.
  digitalWrite(ROW_EN, HIGH);
  digitalWrite(COL_EN, HIGH);

  // Clear shift register
  digitalWrite(RC_CLEAR, LOW);
  digitalWrite(RC_CLEAR, HIGH);

  digitalWrite(ROW_EN, LOW);
  digitalWrite(COL_EN, LOW);

  // Write shift register to output
  digitalWrite(ROW_REG_CLK, LOW);
  digitalWrite(ROW_REG_CLK, HIGH);
  digitalWrite(COL_REG_CLK, LOW);
  digitalWrite(COL_REG_CLK, HIGH);

}

// Write 0b10 pairs (off) into colVec
void colVecOff() {
  for (byte c=0;  c<8;  c++) {
    colVec[c] = 0b10101010;
  }
}

void shiftColVec() {
  digitalWrite(COL_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<8;  i++) {
    SPI.transfer(colVec[i]);
  }
  SPI.endTransaction();
 	digitalWrite(COL_REG_CLK, HIGH);
}


// Turn off all drivers
void allRowsOff() {
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  digitalWrite(RC_SHIFT_CLK, LOW);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b10101010);
  }
  SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allColsOff() {
  colVecOff();
  shiftColVec();
}

// Set all pixels in a column at once.  LSB of rowbits is top row (0).
void setColumn(byte col, uint16_t rowbits) {

  uint32_t hiRowVec = 0;
  uint32_t loRowVec = 0;

  // Build hiRowVec and loRowVec together.
  // Encode and shift from MSB of rowbits into LSB of rowvecs.
  for (byte i=0;  i<16;  i++) {
    hiRowVec <<= 2;
    loRowVec <<= 2;
    if ((rowbits & 0b1000000000000000) != 0) {
      // row high
      hiRowVec &= 0b11111111111111111111111111111100;

      loRowVec |= 0b00000000000000000000000000000010;
      loRowVec &= 0b11111111111111111111111111111110;
    }
    else {
      // Row low.
      hiRowVec |= 0b00000000000000000000000000000010;
      hiRowVec &= 0b11111111111111111111111111111110;

      loRowVec |= 0b00000000000000000000000000000011;
    }
    rowbits <<= 1;
  }
  // Set column low.
  colVecOff();
  byte colpos = col / 4;
  byte colshift = col % 4;
  byte colbits = (0b00000011 << (colshift << 1));
  colVec[colpos] |= colbits;

  // Turn on the ones.
  shiftColVec();
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(hiRowVec & 0xff);
    hiRowVec >>= 8;
  }
	digitalWrite(ROW_REG_CLK, HIGH);

  delayMicroseconds(150);

  allColsOff();

  // Turn off the zeroes.
  colVec[colpos] &= ~colbits;
  shiftColVec();

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(loRowVec & 0xff);
    loRowVec >>= 8;
  }
	digitalWrite(ROW_REG_CLK, HIGH);

  delayMicroseconds(150);

  allColsOff();
  allRowsOff();
}


void setPixel(uint8_t row, uint8_t col, bool on) {
  byte rowvec[4];

  for (int i=0; i<4; i++) {
    rowvec[i] = 0b10101010;
  }
  byte rowpos = row / 4;
  byte rowshift = row % 4;
  byte rowbits = (0b00000011 << (rowshift << 1));

  colVecOff();

  byte colpos = col / 4;
  byte colshift = col % 4;
  byte colbits = (0b00000011 << (colshift << 1));

  if (on) {
    // Row high, col low
    rowvec[rowpos] &= ~rowbits ;
    colVec[colpos] |= colbits;
  }
  else {
    // Row low, col high
    rowvec[rowpos] |= rowbits;
    colVec[colpos] &= ~colbits;
  }

  shiftColVec();

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(rowvec[n]);
  }
	digitalWrite(ROW_REG_CLK, HIGH);

  delayMicroseconds(200);

  allColsOff();
  allRowsOff();

  //delay(5);
}

void setPixel(coord c, bool on) {
  setPixel(c.Y, c.X, on);
}


byte joystick() {

    if (nunchuk_read()) {

        if (nunchuk_joystickX() < -50) {
          return LEFT;
        }
        if (nunchuk_joystickX() > 50) {
          return RIGHT;
        }
        if (nunchuk_joystickY() < -50) {
          return DOWN;
        }
        if (nunchuk_joystickY() > 50) {
          return UP;
        }
        if (nunchuk_buttonZ() > 0) {
          return BUTTON;
        }
    }

    return NONE;
}

void clearPanel() {
      for (byte col=0;  col<30;  col++) {
          setColumn(col, 0b0101010101010101);
      }
      delay(250);
      for (byte col=0;  col<30;  col++) {
          setColumn(col, 0b1010101010101010);
      }
      delay(250);
      for (byte col=0;  col<30;  col++) {
          setColumn(col, 0b0000000000000000);
      }
}

void showSnake() {
  for (byte n=0;  n<snakelen;  n++) {
    setPixel(snake[n], true);
  }
}

// Move the head of the snake, optionally getting longer.
// Returns true if move is OK, or false on badness.
bool moveSnake(byte direction, bool enlongen) {

  coord head = snake[snakelen-1];

  if (enlongen && snakelen == MAX_SNAKE) {
    // Cancel enlongen.
    enlongen = false;
  }

  if (!enlongen) {
    // Turn off tail end
    setPixel(snake[0], false);

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
        return false;
      head.Y = head.Y-1;
      break;
    case DOWN:
      if (head.Y == rowCount-1)
        return false;
      head.Y = head.Y+1;
      break;
    case LEFT:
      if (head.X == 0)
        return false;
      head.X = head.X-1;
      break;
    case RIGHT:
      if (head.X == colCount-1)
        return false;
      head.X = head.X+1;
      break;
  }

  snake[snakelen-1] = head;
  setPixel(head, true);

  // Detect self-crossing snakeness: is head already in rest of body?
  for (byte n=0;  n<snakelen-1;  n++) {
    if (head.X == snake[n].X  &&  head.Y == snake[n].Y)
      return false;
  }
  
  return true;
}

// Wait until a joystick action, then return it. 
byte awaitAction() {
  byte action = NONE;

  while(action == NONE) {
    action = joystick();
  }

  return action;
}

// Play a single game.
void playGame() {
  Serial.println("Play snake!");

  showSnake();

  byte prevAction = awaitAction();
  bool playing = true;
  byte longen = 1;

  while (playing) {

    byte action = joystick();
    if (action == NONE) {
      action = prevAction;
    }

    // Limit direction to right angle moves (no backsies)
    switch (action) {
      case UP:
        if (prevAction == DOWN)
          action = prevAction;
        break;
      case DOWN:
        if (prevAction == UP)
          action = prevAction;
        break;
      case LEFT:
        if (prevAction == RIGHT)
          action = prevAction;
        break;
      case RIGHT:
        if (prevAction == LEFT)
          action = prevAction;
    }

    if (++longen > 10) {
      longen = 0;
    }

    playing = moveSnake(action, longen == 0);
    prevAction = action;

    delay(moveDelay);
  }
}

void showScores() {
  Serial.print("Your score: ");
  Serial.println(score);

  if (score > highScore) {
    highScore = score;
  }
  Serial.print("High score: ");
  Serial.println(highScore);
}
void setup() {

    SPI.begin();
    setupPanel();

    Serial.begin(115200);
    Wire.begin();
    nunchuk_init_power(); // A1 and A2 is power supply
    nunchuk_init();

    clearPanel();
}

void loop() {
    resetGame();
    clearPanel();
    playGame();
    showScores();
} 