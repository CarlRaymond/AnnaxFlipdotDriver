#include <FlipdotPanel.h>
#include <SPI.h>

FlipdotPanel flipdots;

void setup() {
	Serial.begin(115200);
	flipdots.begin();
	SPI.begin();

	Serial.print("No. of panels: ");
	Serial.println(flipdots.panelCount);

	Serial.print("No. of columns: ");
	Serial.println(flipdots.columnCount);
}

void loop() {

	byte row = random(16);
	byte col = random(flipdots.columnCount);

	flipdots.setPixel(row, col, true);
	//delay(10);

	row = random(16);
	col = random(flipdots.columnCount);
	flipdots.setPixel(row, col, false);
	//delay(10);
}