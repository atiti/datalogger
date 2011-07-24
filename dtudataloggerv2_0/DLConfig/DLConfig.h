#ifndef DLConfig_h
#define DLConfig_h

#include "WProgram.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WConstants.h"
#include <DLCommon.h>
#include <DLSD.h>
#include <DLAnalog.h>

typedef struct {
	uint16_t id;
//	char secret[8];
//	char GPRS_apn[15];
//	char GPRS_user[10];
//	char GPRS_pass[10];
//	char HTTP_baseurl[50];
//	char HTTP_status[10];
//	char HTTP_upload[10];
} Config;

class DLConfig
{
	public:
		DLConfig();
		void init(Config *config, DLSD *sd, DLAnalog *analog, char *buff, int len);
		int log_process_callback(char *line, int len);
		uint8_t load();
	private:
		Config *_config;
		DLSD *_sd;
		DLAnalog *_analog;
		char *_buff;
		int _buff_size;
};

#endif

