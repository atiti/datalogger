#ifndef DLSD_h
#define DLSD_h

#include <Arduino.h>
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Fat16.h>
#include <string.h>

#define CONFIG 0
#define DATALOG 1
#define SYSLOG 2
#define SERIALLOG 3
#define DATALOG_READONLY 4

#define NUM_FILES 5

class DLSD
{
	public:
		DLSD(char fullspeed, uint8_t CS_pin);
		void debug(int v);
		int8_t init();
		uint8_t get_num_files();
		uint16_t get_files_count(uint8_t fid);
		uint8_t set_files_count(uint8_t fid, uint16_t count);
		void reset_files_count();
		uint16_t get_saved_count(uint8_t fid);
		uint8_t set_saved_count(uint8_t fid, uint16_t count);
		void reset_saved_count();
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
		char _fullspeed;
		uint8_t _CS;
		uint8_t _DEBUG;
		int8_t _inited;
		SdCard _card;
		Fat16 _files[NUM_FILES];
		boolean _files_open[NUM_FILES];
		uint16_t _files_count[NUM_FILES];
		uint16_t _saved_count[NUM_FILES];
		char _filename[12];
};

#endif

