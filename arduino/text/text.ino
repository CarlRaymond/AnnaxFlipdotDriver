#include <FlipdotPanel.h>
#include <SPI.h>
#include "ascii.h"

FlipdotPanel flipdots;

// Dynamically sized in setup
uint16_t *columns = 0;
byte columnCount = 0;

void shift() {
	for (byte n=0;  n<columnCount-1;  n++) {
		columns[n] = columns[n+1];
	}
	columns[columnCount-1] = 0;
}

void write() {
	for (byte n=0;  n<=columnCount-1;  n++) {
		flipdots.setColumn(n, columns[n]);
	}
}

void clear() {
	for (byte n=0;  n<columnCount-1;  n++) {
		columns[n] = 0x0000;
	}
}

void display(char text[], byte top) {

	int len = strlen(text);

	for (int n=0;  n<len;  n++) {
		byte c = text[n] - 32;
		for (byte i=0;  i<5;  i++) {
			int index = c*5 + i;
			uint16_t data = Font5x7[index] << top;
			shift();
			columns[columnCount-1] = data;
			write();
		}
		shift();
		columns[columnCount-1] = 0;
		write();
	}
}

char hello[] = "Hello";
char thanks[] = "Thanks for watching!  ";
char watch[] = "Watch ";
char appeal[] = "Please LIKE and SUBSCRIBE";

char flipmaster[] = "FlipMaster 9000  "; 
char part1[] = "Part 1  ";
char part2[] = "Part 2  ";
char part3[] = "Part 3  ";
char part4[] = "Part 4  ";
char part5[] = "Part 5  ";

void setup() {
	Serial.begin(115200);
	flipdots.begin();
	SPI.begin();

	Serial.print("No. of panels: ");
	Serial.println(flipdots.panelCount);

	Serial.print("No. of columns: ");
	Serial.println(flipdots.columnCount);
	
	columnCount = flipdots.columnCount;
	columns = new uint16_t[columnCount];
	for (byte i=0;  i<columnCount;  i++) {
		columns[i] = 0;
	}

	flipdots.setAllColumns(0x0000);
	delay(2000);

	display(hello, 5);

	delay(5000);

	display(thanks, 0);
	display(watch, 8);
	display(part2, 8);
	display(appeal, 5);
	delay(500);

	for (byte i=0;  i<3;  i++) {
		flipdots.setAllColumns(0x0000);
		delay(250);
		write();
		delay(250);
	}

	delay(2000);
	clear();
	write();
	delay(2000);

	display(thanks, 0);
	display(watch, 8);
	display(part3, 8);
	display(appeal, 5);
	delay(500);

	for (byte i=0;  i<3;  i++) {
		flipdots.setAllColumns(0x0000);
		delay(250);
		write();
		delay(250);
	}

	delay(2000);
	clear();
	write();
	delay(2000);

	display(thanks, 0);
	display(watch, 8);
	display(part4, 8);
	display(appeal, 5);
	delay(500);

	for (byte i=0;  i<3;  i++) {
		flipdots.setAllColumns(0x0000);
		delay(250);
		write();
		delay(250);
	}

	delay(2000);
	clear();
	write();
	delay(2000);

	display(thanks, 0);
	display(watch, 8);
	display(part5, 8);
	display(appeal, 5);
	delay(500);

	for (byte i=0;  i<3;  i++) {
		flipdots.setAllColumns(0x0000);
		delay(250);
		write();
		delay(250);
	}

	delay(2000);
	clear();
	write();
	delay(2000);

	display(thanks, 0);
	display(appeal, 5);
	delay(500);

	for (byte i=0;  i<3;  i++) {
		flipdots.setAllColumns(0x0000);
		delay(250);
		write();
		delay(250);
	}

	delay(2000);
	clear();
	write();
	delay(2000);


	display(flipmaster, 5);
	display(part1, 5);

	delay(2000);
	clear();
	write();
	delay(2000);

	display(flipmaster, 5);
	display(part2, 5);

	delay(2000);
	clear();
	write();
	delay(2000);

	display(flipmaster, 5);
	display(part3, 5);

	delay(2000);
	clear();
	write();
	delay(2000);

	display(flipmaster, 5);
	display(part4, 5);
	
	delay(2000);
	clear();
	write();
	delay(2000);

	display(flipmaster, 5);
	display(part5, 5);
	delay(2000);
	clear();
	write();

}

void loop() {
}