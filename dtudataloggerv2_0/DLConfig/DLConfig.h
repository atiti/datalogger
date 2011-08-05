#ifndef DLConfig_h
#define DLConfig_h

#include "WProgram.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "WConstants.h"
#include <DLCommon.h>
#include <DLSD.h>
#include <DLAnalog.h>

typedef struct {
	uint16_t id;
	uint32_t http_status_time;
	uint32_t http_upload_time;
	uint8_t measure_time;
	uint16_t measure_length;
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
		void init(DLSD *sd, DLAnalog *analog, char *buff, int len);
		int log_process_callback(char *line, int len);
		uint8_t load();
		uint8_t load_string_EEPROM(uint16_t addr, char *data, int len);
		uint8_t load_EEPROM(uint16_t addr, char *data, int len);
		uint8_t save_EEPROM(uint16_t addr, char *data, int len);
		uint8_t sync_EEPROM(uint16_t addr, char *data, int len);		
		uint8_t save_files_count(uint8_t saved);
		uint8_t load_files_count(uint8_t saved);
		uint8_t load_APN(char *dst, int len);
		uint8_t load_URL(char *dst, int len);
		Config* get_config();
	private:
		Config *_config;
		DLSD *_sd;
		DLAnalog *_analog;
		char *_buff;
		int _buff_size;
};

#endif

