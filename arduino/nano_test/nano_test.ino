#include <SPI.h>
#include <avr/wdt.h>

SPISettings SPImode = SPISettings(1000000, LSBFIRST, SPI_MODE0);

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

// Panel parameters. Updated by senseColumns.
byte panelCount = 1;

// Max panels
byte gapPos[10];


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
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
	SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allRowsHigh() {
  digitalWrite(ROW_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
	SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allRowsLow() {
  digitalWrite(ROW_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
	SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allColsOff() {
  digitalWrite(COL_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
	SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);
}

void allColsLow() {
  digitalWrite(COL_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
  SPI.transfer(0b11111111);
	SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);
}

void allColsHigh() {
  digitalWrite(COL_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
	SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);
}

void flipColumnOff(int col) {
  byte colvec[8];
  
  for (int i=0; i<8; i++) {
    colvec[i] = 0b10101010;
  }
  int pos = col / 4;
  int shift = col % 4;
  byte bits = ~(0b00000011 << (shift << 1));
  colvec[pos] = colvec[pos] & bits ;

  Serial.print("pos, shift, bits: ");
  Serial.print(pos);
  Serial.print(" ");
  Serial.print(shift);
  Serial.print(" ");
  Serial.print(bits, BIN);
  Serial.println();

  Serial.print("Colvec: ");
  for (int i = 0;  i<8;  i++) {
    Serial.print("  ");
    Serial.print(colvec[7-i], BIN);
  }
  Serial.println();
  Serial.println();

  // Selected column high
  digitalWrite(COL_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  for (int i=0;  i<8;  i++) {
    SPI.transfer(colvec[i]);
  }
	SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);
  
  // Rows low, in banks
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b11111111);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b11111111);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b11111111);
  SPI.transfer(0b10101010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b11111111);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

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

  Serial.println("Sensing columns");

  // Keep row drive off
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  for (byte i=0;  i<4;  i++) {
    SPI.transfer(0b10101010);
  }
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);

  SPI.end();

  // Set all columns high
  digitalWrite(COL_REG_CLK, LOW);
  digitalWrite(RC_CLEAR, LOW);
  digitalWrite(RC_CLEAR, HIGH);
  digitalWrite(COL_REG_CLK, HIGH);

  // Set all columns low
  // digitalWrite(COL_REG_CLK, LOW);
  // SPI.beginTransaction(SPImode);
  // for (byte i=0;  i<16;  i++) {
  //   SPI.transfer(0b11111111);
  // }
  // SPI.endTransaction();
  // digitalWrite(COL_REG_CLK, HIGH);

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
  int colCount = 0;
  while(colCount < maxColumns && digitalRead(SENSE) == LOW) {
    colCount++;
    shiftColOff();
    delayMicroseconds(250);
  }

  Serial.print("Possible columns: ");
  Serial.println(colCount);

  // Find all enabled columns.
  shiftColHigh();
  for (int col=colCount;  col>0;  col--) {
    Serial.print("Col. ");
    Serial.print(col);

    if (digitalRead(SENSE) == LOW) {
      Serial.println(": found");
    }
    else {
      Serial.println(": NOT found");
    }

    // Shift two bits to turn this column off, and the next on. 
    shiftColOff();
    delayMicroseconds(250);
  }

  allColsOff();

  // Disable sense
  digitalWrite(SENSE_EN, LOW);
}



void flipColumnOn(int col) {
  byte colvec[8];
  
  for (int i=0; i<8; i++) {
    colvec[i] = 0b10101010;
  }
  int pos = col / 4;
  int shift = col % 4;
  byte bits = (0b00000011 << (shift << 1));
  colvec[pos] = colvec[pos] | bits ;

  Serial.print("pos, shift, bits: ");
  Serial.print(pos);
  Serial.print(" ");
  Serial.print(shift);
  Serial.print(" ");
  Serial.print(bits, BIN);
  Serial.println();

  Serial.print("Colvec: ");
  for (int i = 0;  i<8;  i++) {
    Serial.print("  ");
    Serial.print(colvec[7-i], BIN);
  }
  Serial.println();
  Serial.println();
  
  // Selected column low
  digitalWrite(COL_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  for (int i=0;  i<8;  i++) {
    SPI.transfer(colvec[i]);
  }
	SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);

  // Rows high, in banks
  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b00000000);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b00000000);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b00000000);
  SPI.transfer(0b10101010);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  digitalWrite(ROW_REG_CLK, LOW);
  SPI.beginTransaction(SPImode);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b10101010);
  SPI.transfer(0b00000000);
  SPI.endTransaction();
  digitalWrite(ROW_REG_CLK, HIGH);
  delayMicroseconds(125);

  allRowsOff();
  allColsOff();
}


void setPixel(uint8_t row, uint8_t col, bool on) {
  byte rowvec[4];
  byte colvec[16];

  // Skip the gap
  //if (col > 29)
  //  col += 2;

  for (int i=0; i<4; i++) {
    rowvec[i] = 0b10101010;
  }
  byte rowpos = row / 4;
  byte rowshift = row % 4;
  byte rowbits = (0b00000011 << (rowshift << 1));

  for (int i=0; i<16; i++) {
    colvec[i] = 0b10101010;
  }
  byte colpos = col / 4;
  byte colshift = col % 4;
  byte colbits = (0b00000011 << (colshift << 1));

  if (on) {
    // Row high, col low
    rowvec[rowpos] &= ~rowbits ;
    colvec[colpos] |= colbits;
  }
  else {
    // Row low, col high
    rowvec[rowpos] |= rowbits;
    colvec[colpos] &= ~colbits;
  }

  digitalWrite(COL_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  for (byte n=0;  n<8;  n++) {
    SPI.transfer(colvec[n]);
  }
	SPI.endTransaction();
	digitalWrite(COL_REG_CLK, HIGH);

  digitalWrite(ROW_REG_CLK, LOW);
	SPI.beginTransaction(SPImode);
  for (byte n=0;  n<4;  n++) {
    SPI.transfer(rowvec[n]);
  }
	SPI.endTransaction();
	digitalWrite(ROW_REG_CLK, HIGH);

  delayMicroseconds(200);

  allColsOff();
  allRowsOff();

  //delayMicroseconds(2000);
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
  Serial.println();
  Serial.println();

  SPI.begin();

  allRowsOff();
  allColsOff();

  senseColumns();
}

void loop() {

  const byte colCount = 30;

  byte row;
  byte col;

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

  for (row=0;  row< 16;  row++) {
    for (col=0;  col<colCount;  col++) {
      setPixel(15-row, colCount-1-col, true);
    }
  }
  for (row=0; row<16; row++) {
    for (col=0; col<colCount;  col++) {
      setPixel(15-row, colCount-1-col, false);
    }
  }

  delay(500);

  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(15-row, colCount-1-col, true);
    }
  }
  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(15-row, colCount-1-col, false);
    }
  }

  delay(500);

  for (row=0;  row<16;  row++) {
    for (col=0;  col<colCount;  col++) {
      setPixel(row, col, (row + col) & 0x01 );
    }
  }

  delay(500);

  for (row=0;  row<16;  row++) {
    for (col=0;  col<colCount;  col++) {
      setPixel(row, col, (row + col + 1) & 0x01);
      //delay(5);
    }
  }
  delay(500);

  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, (row + col) >> 2 & 0x01);
    }
  }

  delay(500);

  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, ~((row + col) >> 2) & 0x01);
      }
    }

  delay(500);

  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, (colCount +row - col) >> 2 & 0x01);
    }
  }

  delay(500);

  for (col=0;  col<colCount;  col++) {
    for (row=0;  row<16;  row++) {
      setPixel(row, col, ~((colCount + row - col) >> 2) & 0x01);
      }
    }

  delay(500);

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

}
