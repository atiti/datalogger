#ifndef DLSD_h
#define DLSD_h

#include "WProgram.h"
#include "WConstants.h"
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <SdFat.h>
#include <string.h>

#define CONFIG 0
#define SYSLOG 1
#define DATALOG 2
#define EVENTLOG 3

#define NUM_FILES 4

class DLSD
{
	public:
		DLSD(bool fullspeed, uint8_t CS_pin);
		void debug(int v);
		int8_t init();
		int8_t is_available();
		void pad_filename(char *filename, uint16_t c);
		bool increment_file(uint8_t n);
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
		uint8_t _CS;
		uint8_t _DEBUG;
		int8_t _inited;
		Sd2Card _card;
		SdVolume _vol;
		SdFile _root;
		SdFile _files[NUM_FILES];
		boolean _files_open[NUM_FILES];
		uint16_t _files_count[NUM_FILES];
		char _filename[12];
};

#endif

