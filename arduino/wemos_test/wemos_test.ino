#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "LansingMakersNetwork";
const char* password = "MeetYourMaker2013";

#define COL_REG_CLK D1
#define ROW_REG_CLK D3
#define COL_EN D2
#define ROW_EN D4
#define SENSE_EN D8
#define RC_CLEAR D6
#define RC_DATA D7
#define RC_SHIFT_CLK D0
#define SENSE D5

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
  digitalWrite(RC_SHIFT_CLK, LOW);

  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allRowsHigh() {
  digitalWrite(ROW_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);

  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allRowsLow() {
  digitalWrite(ROW_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);

  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
	digitalWrite(ROW_REG_CLK, HIGH);
}

void allColsOff() {
  digitalWrite(COL_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);

  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b10101010);
	digitalWrite(COL_REG_CLK, HIGH);
}

void allColsLow() {
  digitalWrite(COL_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);

  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b11111111);
	digitalWrite(COL_REG_CLK, HIGH);
}

void allColsHigh() {
  digitalWrite(COL_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);

  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
  shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, 0b00000000);
	digitalWrite(COL_REG_CLK, HIGH);
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
  allRowsOff();

  // Reset column register to force all columns high
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


void setPixel(uint8_t row, uint8_t col, bool on) {
  byte rowvec[4];
  byte colvec[16];

  // Skip the gap
  if (col > 29)
    col += 2;

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
  digitalWrite(RC_SHIFT_CLK, LOW);

  for (byte n=0;  n<16;  n++) {
    shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, colvec[n]);
  }
	digitalWrite(COL_REG_CLK, HIGH);

  digitalWrite(ROW_REG_CLK, LOW);
  digitalWrite(RC_SHIFT_CLK, LOW);
 for (byte n=0;  n<4;  n++) {
    shiftOut(RC_DATA, RC_SHIFT_CLK, LSBFIRST, rowvec[n]);
  }
	digitalWrite(ROW_REG_CLK, HIGH);

  delayMicroseconds(200);

  allColsOff();
  allRowsOff();

  //delayMicroseconds(2000);
}

void setupWifi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready.");
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}

void setup() {

  ESP.wdtDisable();

  pinMode(COL_REG_CLK, OUTPUT);
  pinMode(ROW_REG_CLK, OUTPUT);
  pinMode(COL_EN, OUTPUT);
  pinMode(ROW_EN, OUTPUT);
  pinMode(RC_CLEAR, OUTPUT); // Pin 6
  pinMode(RC_DATA, OUTPUT); // Pin 7
  pinMode(RC_SHIFT_CLK, OUTPUT);
  pinMode(SENSE_EN, OUTPUT);

  pinMode(SENSE, INPUT); // Pin 5

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

  setupWifi();


  allRowsOff();
  allColsOff();

  senseColumns();
}

/*
void loop() {
  int i = 0;

  while(1) {
    i++;
    Serial.println(i);
    delay(1000);
    ESP.wdtFeed();
  }
}
*/


void loop() {
    ESP.wdtFeed();

  const byte colCount = 30;

  byte row;
  byte col;

  Serial.println("On");
  for (row=0;  row< 16;  row++) {
    for (col=0;  col<colCount;  col++) {
      setPixel(row, col, true);
    }
  }
  Serial.println("Off");
  for (row=0; row<16; row++) {
    for (col=0; col<colCount;  col++) {
      setPixel(row, col, false);
    }
  }

  delay(500);

/*
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
*/

}
