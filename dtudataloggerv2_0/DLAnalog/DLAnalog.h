#ifndef DLAnalog_h
#define DLAnalog_h

#include "WProgram.h"

#define VREF 5.0

class DLAnalog
{
	public:
		DLAnalog(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t en, uint8_t inp, bool pullup);
		void debug(int v);
		void enable();
		void disable();
		short read(int pin);
		float read_all(unsigned short *vals);
		void set_pin(int pin, int doa);
		float get_voltage(unsigned short *vals, int pin);
	private:
		uint8_t _en, _inp;
		bool _pullup;
		uint8_t _DEBUG;
		uint8_t _s[4];
		uint8_t _AOD[16];
};

#endif

