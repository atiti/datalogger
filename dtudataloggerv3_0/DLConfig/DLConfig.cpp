#include <Arduino.h>
#include "DLConfig.h"

#define DEVICE_ID_ADDR 0
#define FILES_COUNT_0 2
#define FILES_COUNT_1 4
#define FILES_COUNT_2 6
#define FILES_COUNT_3 8
#define FILES_COUNT_4 10
#define SAVED_COUNT_0 12
#define SAVED_COUNT_1 14
#define SAVED_COUNT_2 16
#define SAVED_COUNT_3 18
#define SAVED_COUNT_4 20
#define APN_STRING 22
#define APN_STRING_LEN 50
#define HTTP_STRING 80
#define HTTP_STRING_LEN 50

Config _int_config;

char* fforward(char *ptr) {
        int i;
        char *cptr = ptr;
        for(i = 0; i < strlen(ptr); i++) {
                if (ptr[i] == ' ')
                        cptr++;
                else
                        return cptr;
        }
        return NULL;
}

DLConfig::DLConfig()
{
	_config = NULL;
	_sd = NULL;
}

void DLConfig::init(DLSD *sd, DLMeasure *measure, char *buff, int len) {
	_config = &_int_config;
	_sd = sd;
	_measure = measure;
	_buff = buff;
	_buff_size = len-1;
	_config->APN = _epc.APN;
	_config->HTTP_URL = _epc.HTTP_URL;
	_config->wdt_events = &_epc.wdt_events;
	_config->eeprom_events = &_epc.eeprom_events;
}

int DLConfig::log_process_callback(char *line, int len) {
        char *param, *ptr;
	uint16_t tmpvar;
        //. Here we extract the name and value combination by finding the equal sign
        ptr = strstr(line, "=");
        param = ptr + 1;
        *ptr = 0;

        //Serial.println(line);
        if (strncmp_P(line, PSTR("PORT"), 4) == 0) {
                ptr = line+10;
                if (strncmp_P(line, PSTR("PORT_MOD"), 8) == 0) {
			tmpvar = atoi(ptr);
			Serial.print(tmpvar);
			Serial.print(": ");
			Serial.println(param);
			if (param[1] == 'A') { // Analog
				_measure->set_pin(tmpvar, IO_ANALOG); 
			} else if (param[1] == 'C') { // Counter
				_measure->set_pin(tmpvar, IO_COUNTER);
			} else if (param[1] == 'D') { // Digital
				_measure->set_pin(tmpvar, IO_DIGITAL);
			} else if (param[1] == 'E') { // Event
				_measure->set_pin(tmpvar, IO_EVENT);
			}
                }
        } else if (strncmp_P(line, PSTR("ID"), 2) == 0) {
		tmpvar = atoi(param);
		_config->id = tmpvar;
		get_from_flash_P(PSTR("DEV ID: "), _buff);
		Serial.print(_buff);
		Serial.println(tmpvar);
		_epc.id = tmpvar;
	} else if (strncmp_P(line, PSTR("ME"), 2) == 0) { // Measurement params
		if (line[8] == 'T') {  // MEASURE_TIME
			_config->measure_time = atoi(param);					
			get_from_flash_P(PSTR("MESTIME: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->measure_time, DEC);
			_measure->set_measure_time(_config->measure_time);	
		} else if (line[8] == 'L') { // MEAUSURE_LENGTH
			_config->measure_length = atoi(param);
			get_from_flash_P(PSTR("MESLEN: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->measure_length, DEC);
		}
	} else if (strncmp_P(line, PSTR("HT"), 2) == 0) { // HTTP params
		if (line[5] == 'U' && line[6] == 'R') { // HTTP_URL
			param = fforward(param);
			if (param != NULL) {
				Serial.println(param);
				*(_epc.HTTP_URL) = '\0'; 
				strncat(_epc.HTTP_URL, param, strlen(param));
			}
		} else if (line[5] == 'S') { // HTTP_STATUS_TIME
			_config->http_status_time = atol(param)*60*1000;
			get_from_flash_P(PSTR("HST: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->http_status_time, DEC);
		} else if (line[5] == 'U' && line[6] == 'P') { // HTTP_UPLOAD_TIME
			_config->http_upload_time = atol(param)*60*1000;
			get_from_flash_P(PSTR("HUT: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->http_upload_time, DEC);
		}
	} else if (strncmp_P(line, PSTR("GP"), 2) == 0) { // GPRS params
		if (line[5] == 'A') { // GPRS_APN
			param = fforward(param);
			if (param != NULL) {
				*(_epc.APN) = '\0';
				strncat(_epc.APN, param, strlen(param));
			}
		} else if (line[5] == 'U') { // GPRS_USER
		} else if (line[5] == 'P') { // GPRS_PASS
		}
	}
        return 0;
}

uint8_t DLConfig::load() {
	int32_t filesize = 0;
	int i;
	load_config_EEPROM(&_epc);
	print_config_EEPROM(&_epc);
	if (_sd->is_available() > 0) {
		filesize = _sd->open(CONFIG, O_READ);
		if (filesize == -1)
			return 0;
		_sd->rewind(CONFIG);
		int rv = 0;
		do {
			rv = _sd->read(CONFIG, _buff, _buff_size, '\n');
			if (_buff[0] != '#' && _buff[0] != 0 && _buff[0] != '\n') {
				log_process_callback(_buff, _buff_size);
			}
		} while (rv >= 0);
		_sd->close(CONFIG);

		if (_epc.wdt_events == _UINT16_MAX_)
			_epc.wdt_events = 0;
		if (_epc.eeprom_events == _UINT16_MAX_)
			_epc.eeprom_events = 0;
		for(i=0;i<NUM_FILES;i++) {
			if (_epc.files_count[i] == _UINT16_MAX_)
				_epc.files_count[i] = 0;
			if (_epc.saved_count[i] == _UINT16_MAX_)
				_epc.saved_count[i] = 0;
		}

		//sync_config_EEPROM(&_epc);
		save_config_EEPROM(&_epc);
		return 1;
	} else {
		_sd->init();
		return 0;
	}
}

uint8_t DLConfig::load_config_EEPROM(EEPROM_config_t *epc) {
	unsigned long checksum = 0L;
	load_EEPROM(0, (char *)epc, sizeof(EEPROM_config_t));
	checksum = crc_struct((char *)epc, sizeof(EEPROM_config_t)-sizeof(unsigned long));

	if (checksum == epc->checksum) {
	} else {
		Serial.println("Checksum failed!");
		epc->eeprom_events++;
	}
}

uint8_t DLConfig::save_config_EEPROM(EEPROM_config_t *epc) {
	unsigned long checksum = 0L;
	checksum = crc_struct((char *)epc, sizeof(EEPROM_config_t)-sizeof(unsigned long));
	epc->checksum = checksum;

	save_EEPROM(0, (char *)epc, sizeof(EEPROM_config_t));
}

uint8_t DLConfig::sync_config_EEPROM(EEPROM_config_t *epc) {
	unsigned long checksum = 0L;
	EEPROM_config_t tepc;
	int i;
	char *p1 = (char *)epc;
	char *p2 = (char *)&tepc;

	checksum = crc_struct((char *)epc, sizeof(EEPROM_config_t)-sizeof(unsigned long));
	epc->checksum = checksum;
	load_config_EEPROM(&tepc);
	if (tepc.checksum != checksum) {
		for(i=0;i<sizeof(EEPROM_config_t);i++) {
			if (p1[i] != p2[i]) {
				EEPROM.write(i, p1[i]);
			}
		}
		return 1;
	}
	return 0;
}

uint8_t DLConfig::print_config_EEPROM(EEPROM_config_t *epc) {
	int i = 0;
	Serial.print("ID: ");
	Serial.println(epc->id, DEC);
	Serial.print("WDT Ev: ");
	Serial.println(epc->wdt_events, DEC);
	Serial.print("EPR Ev: ");
	Serial.println(epc->eeprom_events, DEC);
	for(i=0;i<NUM_FILES;i++) {
		Serial.print("File cnt: ");
		Serial.println(epc->files_count[i], DEC);
		Serial.print("Saved cnt: ");
		Serial.println(epc->saved_count[i], DEC);
	}
	Serial.print("APN: ");
	Serial.println(epc->APN);
	Serial.print("HTTP URL: ");
	Serial.println(epc->HTTP_URL);
	Serial.print("Checksum: ");
	Serial.println(epc->checksum, HEX);	
}

uint8_t DLConfig::load_string_EEPROM(uint16_t addr, char *data, int len) {
	int j = 0;
	for(j=0;j<len;j++) {
		data[j] = (char)EEPROM.read(addr+j);
		if (data[j] == 0)
			return 1;
	}
	return 1;
}

uint8_t DLConfig::load_EEPROM(uint16_t addr, char *data, int len) {
	int j = 0;
	for(j=0;j<len;j++) {
		data[j] = (char)EEPROM.read(addr+j);
	}
	return 1;
}

uint8_t DLConfig::save_EEPROM(uint16_t addr, char *data, int len) {
	int j = 0;
	for(j=0;j<len;j++) {
		EEPROM.write(addr+j, data[j]);
	}
	return 1;
}

// Sync EEPROM is graceful to the EEPROM by checking before overwriting segments
uint8_t DLConfig::sync_EEPROM(uint16_t addr, char *data, int len) {
	int j = 0;
	uint8_t match = 1;
	load_EEPROM(addr, _buff, len);
	for(j=0;j<len;j++) {
		if (_buff[j] != data[j]) {
			match = 0;
			break;
		}
	}
	if (!match) {
		save_EEPROM(addr, data, len);
		return 1;
	}
	return 0;
}

uint8_t DLConfig::load_files_count(uint8_t saved) {
	uint16_t addr;
	if (saved)
		addr = SAVED_COUNT_0;
	else
		addr = FILES_COUNT_0;
	uint16_t v = 0;
	for(uint8_t i = 0; i < _sd->get_num_files(); i++) {
		Serial.print(saved, DEC);
		Serial.print(" LFC: ");
		if (!saved) {
			Serial.println(_epc.files_count[i]);
			_sd->set_files_count(i, _epc.files_count[i]);
		} else {
			Serial.println(_epc.saved_count[i]);	
			_sd->set_saved_count(i, _epc.saved_count[i]);
		}
	}	
}

uint8_t DLConfig::save_files_count(uint8_t saved) {
        uint16_t addr;
        if (saved)
                addr = SAVED_COUNT_0;
        else
                addr = FILES_COUNT_0;
	uint16_t v = 0;
	for(uint8_t i = 0; i < _sd->get_num_files(); i++) {
		if (!saved) {
			v = _sd->get_files_count(i); // Get the current pos
			_epc.files_count[i] = v;
		} else {
			v = _sd->get_saved_count(i);
			_epc.saved_count[i] = v;
		}
		Serial.print("SFC: ");
		Serial.println(v);
	}
	sync_config_EEPROM(&_epc);
}

Config* DLConfig::get_config() {
	return _config;
}

uint8_t DLConfig::load_APN(char *dst, int len) {
	load_string_EEPROM(APN_STRING, dst, APN_STRING_LEN);
	return 1;
}

uint8_t DLConfig::load_URL(char *dst, int len) {
	load_string_EEPROM(HTTP_STRING, dst, HTTP_STRING_LEN);
	return 1;
}


