#ifndef DLSD_h
#define DLSD_h

#include "WProgram.h"
#include "WConstants.h"
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <SdFat.h>
#include <string.h>

#define DATALOG 0
#define SYSLOG 1
#define CONFIG 2
#define EVENT 3

#define NUM_FILES 4

class DLSD
{
	public:
		DLSD(bool fullspeed, int CS_pin);
		void debug(int v);
		int8_t init();
		unsigned long open(uint8_t n, uint8_t flags);
		bool close(uint8_t n);
		bool write(uint8_t n, char *ptr);
		bool write(uint8_t n, float a);
		bool write(uint8_t n, unsigned short a);
		bool write(uint8_t n, int a);
		bool write(uint8_t n, unsigned long a);
		int read(uint8_t n, char *ptr, int len);
		int read(uint8_t n, char *ptr, int len, char t);
		void rewind(uint8_t n);
		bool seek(uint8_t n, uint32_t pos);
		bool seekend(uint8_t n);
	private:
		bool _fullspeed;
		int _CS;
		uint8_t _DEBUG;
		Sd2Card _card;
		SdVolume _vol;
		SdFile _root;
		SdFile _files[NUM_FILES];
		boolean _files_open[NUM_FILES];
		char _filename[12];
};

#endif

