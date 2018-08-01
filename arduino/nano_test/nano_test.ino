#include <SPI.h>
#include <avr/wdt.h>

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
//#define CTRL_A4
#define CTRL_B0 2
#define CTRL_B1 15
#define CTRL_B2 17
#define CTRL_B3 19
//#define CTRL_B4

SPISettings SPImode = SPISettings(1000000, LSBFIRST, SPI_MODE0);

// Panel parameters. Updated by senseColumns.
byte panelCount = 0;

// Number of addressable (real) columns
byte colCount = 0;

// Dynamically sized in setup
byte *colVec = 0;

// Gaps and adjustments
byte gapStart[4] = { 0, 0, 0, 0 };
byte gapLength[4] = { 0, 0, 0, 0 };
byte gapCount = 0;


// Write 0b10 pairs (off) into colVec
void colVecOff() {
  for (byte c=0;  c<(panelCount*8);  c++) {
    colVec[c] = 0b10101010;
  }
}

void shiftColVec() {
  digitalWrite(COL_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(colVec[i]);
  }
  SPI.endTransaction();
 	digitalWrite(COL_REG_CLK, HIGH);
}


// Corrects a virtual (apparent) column position to a real column position.
byte virtualColToActualCol(byte col) {
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

void allRowsHigh() {
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b00000000);
  }
  SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allRowsLow() {
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b11111111);
  }
  SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allColsOff() {
  colVecOff();
  shiftColVec();
}

void allColsLow() {
  digitalWrite(COL_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(0b11111111);
  }
  SPI.endTransaction();
 	digitalWrite(COL_REG_CLK, HIGH);
}

void allColsHigh() {
  digitalWrite(COL_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(0b00000000);
  }
  SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);
}


void flipColumnOn(byte col) {
  
  colVecOff();

  col = virtualColToActualCol(col);

  int pos = col / 4;
  int shift = col % 4;
  byte bits = (0b00000011 << (shift << 1));
  colVec[pos] = colVec[pos] | bits ;

  // Selected column low
  shiftColVec();

  // Rows high, in banks
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b00100010);
  SPI.transfer(0b00100010);
  SPI.transfer(0b00100010);
  SPI.transfer(0b00100010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(150);

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10001000);
  SPI.transfer(0b10001000);
  SPI.transfer(0b10001000);
  SPI.transfer(0b10001000);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(150);

  allRowsOff();
  allColsOff();
}

void flipColumnOff(byte col) {
  
  colVecOff();

  col = virtualColToActualCol(col);

  int pos = col / 4;
  int shift = col % 4;
  byte bits = ~(0b00000011 << (shift << 1));
  colVec[pos] = colVec[pos] & bits ;

  // Selected column high
  digitalWrite(COL_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (int i=0;  i<(panelCount * 8);  i++) {
    SPI.transfer(colVec[i]);
  }
  SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);
  
  // Rows low, in banks
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b11101110);
  SPI.transfer(0b11101110);
  SPI.transfer(0b11101110);
  SPI.transfer(0b11101110);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(150);

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10111011);
  SPI.transfer(0b10111011);
  SPI.transfer(0b10111011);
  SPI.transfer(0b10111011);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(150);

  allRowsOff();
  allColsOff();

}



// Manually shift 0b10 on the column lines.
void shiftColOff() {
  
  digitalWrite(COL_REG_CLK, LOW);

  // Shift 0 (LSB)
  digitalWrite(RC_SHIFT_CLK, LOW);
  digitalWrite(RC_DATA, LOW);
  delayMicroseconds(100);
  digitalWrite(RC_SHIFT_CLK, HIGH);
  delayMicroseconds(100);

  // Shift 1
  digitalWrite(RC_SHIFT_CLK, LOW);
  digitalWrite(RC_DATA, HIGH);
  delayMicroseconds(100);
  digitalWrite(RC_SHIFT_CLK, HIGH);
  delayMicroseconds(100);

  digitalWrite(COL_REG_CLK, HIGH);
}

// Manually shift 0b00
void shiftColHigh() {
  digitalWrite(COL_REG_CLK, LOW);

  // Shift 0
  digitalWrite(RC_SHIFT_CLK, LOW);
  digitalWrite(RC_DATA, LOW);
  digitalWrite(RC_SHIFT_CLK, HIGH);
  
  // Shift another 0
  digitalWrite(RC_SHIFT_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, HIGH);
  
  digitalWrite(COL_REG_CLK, HIGH);

}

void senseColumns() {

  // No SPI allowed here

  Serial.println("Sensing columns");

  // Keep row drive off
  digitalWrite(ROW_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);

  for (byte i=0;  i<4;  i++) {
    shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  }

  digitalWrite(ROW_REG_CLK, HIGH);

  // Set all columns high
  digitalWrite(COL_REG_CLK, LOW);
  digitalWrite(RC_CLEAR, LOW);
  digitalWrite(RC_CLEAR, HIGH);
  digitalWrite(COL_REG_CLK, HIGH);

  // Enable sense
  digitalWrite(SENSE_EN, HIGH);
  delayMicroseconds(10);
  if (digitalRead(SENSE) == LOW) {
    Serial.println("Sensed!");
  }
  else {
    Serial.println("D'oh");
  }

  const int maxColumns = 255;

  // Find potential columns. While column sensed, shift two bits turning column off.
  byte vColCount = 0;
  while(vColCount < maxColumns && digitalRead(SENSE) == LOW) {
    vColCount++;
    shiftColOff();
    delayMicroseconds(250);
  }

  Serial.print("vColCount: ");
  Serial.println(vColCount);

  // Column iteration is in reverse order; we'll find gaps backwards, and adjust.
  byte revGapStart[4];
  byte revGapLength[4];

  // Gap start position and length
  byte start = 0;
  byte len = 0;
  byte count = 0;

  // Find all actual columns, and record gap positions.
  colCount = 0;
  shiftColHigh();

  for (int col=vColCount;  col>0;  col--) {
    Serial.print("Col. ");
    Serial.print(col);

    if (digitalRead(SENSE) == LOW) {
      Serial.println(": found");
      colCount++;

      if (len > 0) {
        // Just walked past a gap.
        revGapStart[count] = start;
        revGapLength[count] = len;
        count++;
        len = 0;
      }
    }
    else {
      Serial.println(": NOT found");
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
  digitalWrite(SENSE_EN, LOW);

  // Reverse order and adjust gap positions.
  for (byte g=0;  g<count-1;  g++) {
    byte i = count-1-g;
    gapStart[g] = revGapStart[i] - revGapLength[i];
    gapLength[g] = revGapLength[i];
  }
  gapCount = count-1;

  Serial.print("Actual columns: ");
  Serial.println(colCount);

  // Allocate colVec
  colVec = new byte[colCount];

  panelCount = (colCount + 31) / 32;
}


// Set all pixels in a column at once.  LSB of rowbits is top row (0).
void setColumn(byte col, uint16_t rowbits) {

  // Skip the gaps
  col = virtualColToActualCol(col);

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
  //allRowsOff();

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

  // Skip the gaps
  col = virtualColToActualCol(col);

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

void testGapper() {
  for (byte g=0;  g<gapCount;  g++) {
    Serial.print("Gap at ");
    Serial.print(gapStart[g]);
    Serial.print(" length ");
    Serial.println(gapLength[g]);
  }
}

void setup() {

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

  Serial.begin(115200);

  SPI.begin();
  allRowsOff();
  SPI.end();
  senseColumns();
  SPI.begin();

  testGapper();
}

void loop() {

  int8_t row;
  int8_t col;


  // Wipe down
  for (row=0;  row< 16;  row++) {
    for (col=0;  col<colCount;  col++) {
      setPixel(row, col, true);
    }
  }
  for (row=0; row<16; row++) {
    for (col=0; col<colCount;  col++) {
      setPixel(row, col, false);
    }
  }

  delay(500);


  // Wipe right
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, true);
    }
  }
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, false);
    }
  }

  delay(500);

  // Wipe up
  for (row=15;  row>=0;  row--) {
    for (col=colCount-1;  col>=0;  col--) {
      setPixel(row, col, true);
    }
  }
  for (row=15; row>=0; row--) {
    for (col=colCount-1; col>=0;  col--) {
      setPixel(row, col, false);
    }
  }

  delay(500);

  // Wipe left
  for (col=colCount-1;  col>=0;  col--) {
    for (row=15;  row>=0;  row--) {
      setPixel(row, col, true);
    }
  }
  for (col=colCount-1;  col>=0;  col--) {
    for (row=15;  row>=0;  row--) {
      setPixel(row, col, false);
    }
  }

  delay(500);

  // Full checkerboard
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, (row + col) & 0x01 );
    }
  }

  delay(500);

  // Alternate checkerboard
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, (row + col + 1) & 0x01 );
    }
  }

  delay(500);

  // Full checkerboard
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, (row + col) & 0x01 );
    }
  }

  delay(500);

  // Alternate checkerboard
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, (row + col + 1) & 0x01 );
    }
  }

  delay(500);

  // Row flash
  for(byte n=0;  n<8;  n++) {
    for (col=0;  col<colCount;  col++) {
      setColumn(col, (uint16_t)0b0001000100010001);
    }
    for (col=0;  col<colCount;  col++) {
      setColumn(col, (uint16_t)0b0010001000100010);
    }
    for (col=0;  col<colCount;  col++) {
      setColumn(col, (uint16_t)0b0100010001000100);
    }
    for (col=0;  col<colCount;  col++) {
      setColumn(col, (uint16_t)0b1000100010001000);
    }
  }

  delay(500);


  // Slash
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, ~((row + col) >> 2) & 0x01);
    }
  }

  delay(500);
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, ((row + col) >> 2) & 0x01);
    }
  }

  delay(500);

  // Backslash
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, ((colCount +row - col) >> 2) & 0x01);
    }
  }

  delay(500);

  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, ~((colCount + row - col) >> 2) & 0x01);
      }
    }

  delay(500);

  // Horizontal stripes
  for (row=0; row<16; row++) {
    for (col=0; col<colCount; col++) {
      if (row % 3 == 0) {
        setPixel(row, col, true);
      }
      else {
        setPixel(row, col, false);
      }
 
    }
  }

  delay(500);

  // Vertical stripes
  for (row=0; row<16; row++) {
    for (col=0; col<colCount; col++) {
      if (col % 3 == 0) {
        setPixel(row, col, true);
      }
      else {
        setPixel(row, col, false);
      }
    }
  }

  delay(500);

  // Flash all columns
  for (byte n=0;  n<4;  n++) {
    for (col=0;  col<colCount;  col++) {
      flipColumnOn(col);
    }
    delay(15);
    for (col=0;  col<colCount;  col++) {
      flipColumnOff(col);
    }
    delay(15);
  }

  delay(500);

  byte lastOn = 0;
  for (col=0;  col<colCount;  col++) {
    flipColumnOff(lastOn);
    flipColumnOn(col);
    lastOn = col;

    delay(15);
  }
  flipColumnOff(lastOn);

  lastOn = colCount-1;
  for (col=colCount-1;  col>=0;  col--) {
    flipColumnOff(lastOn);
    flipColumnOn(col);
    lastOn = col;

    delay(15);
  }
  flipColumnOff(lastOn);

  delay(500);

  for (col=0;  col<colCount;  col++) {
    flipColumnOn(col);
  }

  lastOn = colCount-1;
  for (col=colCount-1;  col>=0;  col--) {
    flipColumnOn(lastOn);
    flipColumnOff(col);
    lastOn = col;

    delay(15);
  }
  flipColumnOn(lastOn);

  lastOn = 0;
  for (col=0;  col<colCount;  col++) {
    flipColumnOn(lastOn);
    flipColumnOff(col);
    lastOn = col;

    delay(15);
  }
  flipColumnOn(lastOn);

  delay(500);

  for (col=0;  col<colCount;  col++) {
    flipColumnOff(col);
  }

  delay(500);
}
