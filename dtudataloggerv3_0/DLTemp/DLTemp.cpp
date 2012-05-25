#include <Arduino.h>
#include "DLTemp.h"

#define DLTEMP_ANALOGMAX 1023.0

short PROGMEM _temp_const[20] = {990, 1040, 1146, 1260, 1381, 1510,
                               1646, 1790, 1941, 2100, 2267, 2441,
                               2623, 2812, 3009, 3214, 3426, 3643, 3855};


DLTemp::DLTemp(float R1, float VIN, float offset)
{
	_R1 = R1;
	_VIN = VIN;
	_offset = offset;
	_vals = NULL;
}

void DLTemp::init(int pos, unsigned short *vals) {
	_pos = pos;
	_vals = vals;
}

float DLTemp::getTemp() {
	float Vout = 0, temp = 0;
	short R2 = 0;
	int i = 0;
	// Lets abort if no pointer is set
	if (!_vals) return -999;

	Vout = (_VIN / DLTEMP_ANALOGMAX) * _vals[_pos];
	R2 = (short)(((_R1 * ((_VIN / Vout) - 1)) + _offset) * 1000);
	for(i = 0; i < 20; i++) {
		if (pgm_read_word_near(_temp_const + (i-1)) < R2 &&
		    pgm_read_word_near(_temp_const + i) >= R2)
			break;
	}
	temp = (i*10) - 70 + (10 - ((float)(pgm_read_word_near(_temp_const + i) - R2) / (float)(pgm_read_word_near(_temp_const + i) -pgm_read_word_near(_temp_const + (i-1)))) * 10);
	return temp;
}
