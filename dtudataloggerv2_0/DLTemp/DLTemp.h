#ifndef DLTemp_h
#define DLTemp_h

#include "WProgram.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WConstants.h"

// Predefined resistances at temperatures from -60 to 120C
PROMEM short temp_const[20] = {990, 1040, 1146, 1260, 1381, 1510,
			       1646, 1790, 1941, 2100, 2267, 2441,
			       2623, 2812, 3009, 3214, 3426, 3643, 3855};

class DLTemp
{
	public:
		DLTemp(float R1, float VIN, float offset);
		void init(int pos, unsigned short *vals);
		float getTemp();
	private:
		unsigned short *_vals;
		uint8_t _pos;
		float _R1, _VIN, _offset;
};

#endif

