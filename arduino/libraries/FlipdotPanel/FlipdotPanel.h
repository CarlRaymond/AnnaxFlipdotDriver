#include <Arduino.h>

class FlipdotPanel {

	public:

		FlipdotPanel();

		void begin();

		// Panel parameters. Updated by senseColumns.

		// No. of (visible) columns.
		byte columnCount;

		// No. of virtual columns -- corresponds to shift register positions
		byte vColCount;

		// No. of panels.
		byte panelCount = 0;

		// Dynamically sized in setup
		byte *colVec = 0;

		// Convert visible column to shift register position column
		byte virtualToPhysical(byte col);

		void allRowsOff();
		void allRowsHigh();
		void allRowsLow();

		void allColsOff();
		void allColsHigh();
		void allColsLow();

		void setColumn(byte col, uint16_t rowbits);
		void setColumn(byte col, uint16_t rowbits, uint16_t maskbits, bool invert);
		void setAllColumns(uint16_t rowbits);
		void setAllColumns(uint16_t rowbits, uint16_t maskbits, bool invert);
		void setPixel(byte row, byte col, bool on);

	private:

		// Gaps and adjustments
		byte gapStart[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		byte gapLength[16] = { 0, 0, 0, 0,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 , 0, 0 };
		byte gapCount = 0;

		void shiftColOff();
		void shiftColHigh();

		void senseColumns();

		void colVecOff();

		void shiftColVec();


};