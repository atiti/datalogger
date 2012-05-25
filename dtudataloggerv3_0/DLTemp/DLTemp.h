#ifndef DLTemp_h
#define DLTemp_h

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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

