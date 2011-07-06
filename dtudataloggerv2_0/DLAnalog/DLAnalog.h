#ifndef DLAnalog_h
#define DLAnalog_h

#define "WProgram.h"

class DLAnalog
{
	public:
		DLAnalog(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t en, uint8_t inp, bool pullup);
		void debug(int v);
		void enable();
		void disable();
		int read(int pin);
		void readAll();
		void setPin(int pin, int doa);
	private:
		uint8_t _en, _inp;
		bool _pullup;
		bool _DEBUG;
		uint8_t _s[4];
		uint8_t _AOD[16];
};

#endif

