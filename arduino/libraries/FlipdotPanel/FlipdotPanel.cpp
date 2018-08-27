#include "FlipdotPanel.h"
#include <SPI.h>

SPISettings SPImode = SPISettings(2000000, LSBFIRST, SPI_MODE0);

#define COL_REG_CLK_PIN 4
#define ROW_REG_CLK_PIN 5
#define COL_EN_PIN 6
#define ROW_EN_PIN 7
#define SENSE_EN_PIN 8
#define RC_CLEAR_PIN 9
#define RC_DATA_PIN 11
#define SENSE_PIN 12
#define RC_CLK_PIN 13

#define CTRL_A0_PIN 3
#define CTRL_A1_PIN 14
#define CTRL_A2_PIN 16
#define CTRL_A3_PIN 18
#define CTRL_B0_PIN 2
#define CTRL_B1_PIN 15
#define CTRL_B2_PIN 17
#define CTRL_B3_PIN 19

FlipdotPanel::FlipdotPanel() {
}

// Initializer. Set up pins, determine panel size.
void FlipdotPanel::begin() {
	// Configure pins
	pinMode(COL_REG_CLK_PIN, OUTPUT);
	pinMode(ROW_REG_CLK_PIN, OUTPUT);
	pinMode(COL_EN_PIN, OUTPUT);
	pinMode(ROW_EN_PIN, OUTPUT);
	pinMode(RC_CLEAR_PIN, OUTPUT);
	pinMode(RC_DATA_PIN, OUTPUT);
	pinMode(RC_CLK_PIN, OUTPUT);
	pinMode(SENSE_EN_PIN, OUTPUT);
	pinMode(SENSE_PIN, INPUT);

  pinMode(CTRL_A0_PIN, INPUT);
  pinMode(CTRL_A1_PIN, INPUT);
  pinMode(CTRL_A2_PIN, INPUT);
  pinMode(CTRL_A3_PIN, INPUT);
  pinMode(CTRL_B0_PIN, INPUT);
  pinMode(CTRL_B1_PIN, INPUT);
  pinMode(CTRL_B2_PIN, INPUT);
  pinMode(CTRL_B3_PIN, INPUT);

	// Disable sense
	digitalWrite(SENSE_EN_PIN, LOW);

	// Disable outputs -- turns on all row and column lines to HIGH state. No net current flows.
  digitalWrite(ROW_EN_PIN, HIGH);
  digitalWrite(COL_EN_PIN, HIGH);

	// Clear shift register
	digitalWrite(RC_CLEAR_PIN, LOW);
	digitalWrite(RC_CLEAR_PIN, HIGH);

	digitalWrite(ROW_EN_PIN, LOW);
	digitalWrite(COL_EN_PIN, LOW);

	// Write shift register to output by toggling register clocks
	digitalWrite(ROW_REG_CLK_PIN, LOW);
	digitalWrite(ROW_REG_CLK_PIN, HIGH);
	digitalWrite(COL_REG_CLK_PIN, LOW);
	digitalWrite(COL_REG_CLK_PIN, HIGH);

  // Still have all row and column lines HIGH.
  senseColumns();
  SPI.begin();
}


byte FlipdotPanel::virtualToPhysical(byte col) {
  for (byte g=0;  g<gapCount;  g++) {
    if (col >= gapStart[g])
      col += gapLength[g];
  }
  return col;
}

//
// Bit pairs for various outputs:
//
// HL
// 00: HIGH: high side on (pulled high), low side off
// 01: XLOW: diode prevents pullup enabling high side, low side on
// 10: OFF: high side off, low side off
// 11: LOW: high side off, low side on


// Turn off all drivers
void FlipdotPanel::allRowsOff() {
  digitalWrite(ROW_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  digitalWrite(RC_CLK_PIN, LOW);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b10101010);
  }
  SPI.endTransaction();
	digitalWrite(ROW_REG_CLK_PIN, HIGH);
}

void FlipdotPanel::allRowsHigh() {
  digitalWrite(ROW_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b00000000);
  }
  SPI.endTransaction();
	digitalWrite(ROW_REG_CLK_PIN, HIGH);
}

void FlipdotPanel::allRowsLow() {
  digitalWrite(ROW_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b11111111);
  }
  SPI.endTransaction();
	digitalWrite(ROW_REG_CLK_PIN, HIGH);
}


// Write 0b10 pairs (off) into colVec
void FlipdotPanel::colVecOff() {
  for (byte c=0;  c<(panelCount*8);  c++) {
    colVec[c] = 0b10101010;
  }
}

// Shift colVec out.
void FlipdotPanel::shiftColVec() {
  digitalWrite(COL_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(colVec[i]);
  }
  SPI.endTransaction();
 	digitalWrite(COL_REG_CLK_PIN, HIGH);
}


void FlipdotPanel::allColsOff() {
  colVecOff();
  shiftColVec();
}

void FlipdotPanel::allColsLow() {
  digitalWrite(COL_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(0b11111111);
  }
  SPI.endTransaction();
 	digitalWrite(COL_REG_CLK_PIN, HIGH);
}

void FlipdotPanel::allColsHigh() {
  digitalWrite(COL_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(0b00000000);
  }
  SPI.endTransaction();
	digitalWrite(COL_REG_CLK_PIN, HIGH);
}


// Manually shift 0b10 on the column lines.
void FlipdotPanel::shiftColOff() {
  
  digitalWrite(COL_REG_CLK_PIN, LOW);

  // Shift 0 (LSB)
  digitalWrite(RC_CLK_PIN, LOW);
  digitalWrite(RC_DATA_PIN, LOW);
  delayMicroseconds(100);
  digitalWrite(RC_CLK_PIN, HIGH);
  delayMicroseconds(100);

  // Shift 1
  digitalWrite(RC_CLK_PIN, LOW);
  digitalWrite(RC_DATA_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(RC_CLK_PIN, HIGH);
  delayMicroseconds(100);

  digitalWrite(COL_REG_CLK_PIN, HIGH);
}

// Manually shift 0b00
void FlipdotPanel::shiftColHigh() {
  digitalWrite(COL_REG_CLK_PIN, LOW);

  // Shift 0
  digitalWrite(RC_CLK_PIN, LOW);
  digitalWrite(RC_DATA_PIN, LOW);
  digitalWrite(RC_CLK_PIN, HIGH);
  
  // Shift another 0
  digitalWrite(RC_CLK_PIN, LOW);
  digitalWrite(RC_CLK_PIN, HIGH);
  
  digitalWrite(COL_REG_CLK_PIN, HIGH);

}


void FlipdotPanel::senseColumns() {

  // No SPI allowed here.
  // Set rows OFF.
  digitalWrite(ROW_REG_CLK_PIN, LOW);
  digitalWrite(RC_CLK_PIN, LOW);
  for (byte i=0;  i<4;  i++) {
    shiftOut(RC_DATA_PIN, RC_CLK_PIN, LSBFIRST, 0b10101010);
  }
  digitalWrite(ROW_REG_CLK_PIN, HIGH);

  // Set all columns HIGH by clearing column register
  digitalWrite(COL_REG_CLK_PIN, LOW);
  digitalWrite(RC_CLEAR_PIN, LOW);
  digitalWrite(RC_CLEAR_PIN, HIGH);
  digitalWrite(COL_REG_CLK_PIN, HIGH);

  // Enable sense
  digitalWrite(SENSE_EN_PIN, HIGH);
  delayMicroseconds(250);

  const byte maxColumns = 255;

  // Find potential columns. While column sensed, shift two bits turning column off.
  byte physColCount = 0;
  while(physColCount < maxColumns && digitalRead(SENSE_PIN) == LOW) {
    physColCount++;
    shiftColOff();
    delayMicroseconds(250);
  }

  // Column iteration is in reverse order; we'll find gaps backwards, and adjust.
  byte revGapStart[16];
  byte revGapLength[16];

  // Gap start position and length
  byte start = 0;
  byte len = 0;
  byte count = 0;

  // Find all actual columns, and record gap positions.
  columnCount = 0;
  shiftColHigh();

  for (int col=physColCount;  col>0;  col--) {

    if (digitalRead(SENSE_PIN) == LOW) {
      columnCount++;
      if (len > 0) {
        // Just walked past a gap.
        revGapStart[count] = start;
        revGapLength[count] = len;
        count++;
        len = 0;
      }
    }
    else {
      if (len == 0) {
        // New gap
        start = col;
        len = 1;
      }
      else {
        // Continue existing gap
        len++;
      }
    }

    // Shift two bits to turn this column off, and the next on. 
    shiftColOff();
    delayMicroseconds(250);
  }

  // Disable sense
  digitalWrite(SENSE_EN_PIN, LOW);

  // Reverse order and adjust gap positions.
  for (byte g=0;  g<count-1;  g++) {
    byte i = count-1-g;
    gapStart[g] = revGapStart[i] - revGapLength[i];
    gapLength[g] = revGapLength[i];
  }
  if (count > 0) {
    gapCount = count-1;
  }
  else {
    gapCount = 0;
  }

  // Allocate colVec
  colVec = new byte[columnCount];

  panelCount = (columnCount + 31) / 32;
}



// Set all pixels in a column at once.  LSB of rowbits is top row (0).
void FlipdotPanel::setColumn(byte col, uint16_t rowbits) {

  // Skip the gaps
  col = virtualToPhysical(col);

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
  digitalWrite(ROW_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(hiRowVec & 0xff);
    hiRowVec >>= 8;
  }
	digitalWrite(ROW_REG_CLK_PIN, HIGH);

  delayMicroseconds(150);

  allColsOff();
  //allRowsOff();

  // Turn off the zeroes.
  colVec[colpos] &= ~colbits;
  shiftColVec();

  digitalWrite(ROW_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(loRowVec & 0xff);
    loRowVec >>= 8;
  }
	digitalWrite(ROW_REG_CLK_PIN, HIGH);

  delayMicroseconds(150);

  allColsOff();
  allRowsOff();
  
}


void FlipdotPanel::setPixel(byte row, byte col, bool on) {
  byte rowvec[4];

  // Skip the gaps
  col = virtualToPhysical(col);

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

  digitalWrite(ROW_REG_CLK_PIN, LOW);
  SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(rowvec[n]);
  }
	digitalWrite(ROW_REG_CLK_PIN, HIGH);

  delayMicroseconds(150);

  allColsOff();
  allRowsOff();
}
