#ifndef DLConfig_h
#define DLConfig_h

#define _UINT16_MAX_ 0xffff

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <DLCommon.h>
#include <DLSD.h>
#include <DLMeasure.h>

typedef struct {
	uint16_t id;
	uint16_t wdt_events;
	uint16_t eeprom_events;
	uint16_t files_count[NUM_FILES];
	uint16_t saved_count[NUM_FILES];
	char APN[20];
	char HTTP_URL[50];
	unsigned long checksum;
} EEPROM_config_t;

typedef struct {
	uint16_t id;
	uint32_t http_status_time;
	uint32_t http_upload_time;
	uint32_t measure_time;
	uint32_t measure_length;
	char *APN;
	char *HTTP_URL;
	uint16_t *wdt_events;
	uint16_t *eeprom_events;
} Config;

class DLConfig
{
	public:
		DLConfig();
		void init(DLSD *sd, DLMeasure *measure, char *buff, int len);
		int log_process_callback(char *line, int len);
		uint8_t load();
		uint8_t load_config_EEPROM(EEPROM_config_t *epc); 
		uint8_t save_config_EEPROM(EEPROM_config_t *epc);
		uint8_t sync_config_EEPROM(EEPROM_config_t *epc);
		uint8_t print_config_EEPROM(EEPROM_config_t *epc);
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
		EEPROM_config_t _epc;
		Config *_config;
		DLSD *_sd;
		DLMeasure *_measure;
		char *_buff;
		int _buff_size;
};

#endif

