#include "WProgram.h"
#include "DLConfig.h"

#define DEVICE_ID_ADDR 0
#define FILES_COUNT_0 2
#define FILES_COUNT_1 4
#define SAVED_COUNT_0 6
#define SAVED_COUNT_1 8
#define APN_STRING 10
#define APN_STRING_LEN 50
#define HTTP_STRING 60
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

void DLConfig::init(DLSD *sd, DLAnalog *analog, char *buff, int len) {
	_config = &_int_config;
	_sd = sd;
	_analog = analog;
	_buff = buff;
	_buff_size = len-1;
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
				_analog->set_pin(tmpvar, IO_ANALOG); 
			} else if (param[1] == 'C') { // Counter
				_analog->set_pin(tmpvar, IO_COUNTER);
			} else if (param[1] == 'D') { // Digital
				_analog->set_pin(tmpvar, IO_DIGITAL);
			} else if (param[1] == 'E') { // Event
				_analog->set_pin(tmpvar, IO_EVENT);
			}
                }
        } else if (strncmp_P(line, PSTR("ID"), 2) == 0) {
		tmpvar = atoi(param);
		_config->id = tmpvar;
		get_from_flash_P(PSTR("Device id: "), _buff);
		Serial.print(_buff);
		Serial.println(tmpvar);
		if (sync_EEPROM(DEVICE_ID_ADDR, (char *)(&tmpvar), 2))
			get_from_flash_P(PSTR("E W"), _buff);
		else
			get_from_flash_P(PSTR("E OK"), _buff);
		Serial.println(_buff);	
	} else if (strncmp_P(line, PSTR("ME"), 2) == 0) { // Measurement params
		if (line[8] == 'T') {  // MEASURE_TIME
			_config->measure_time = atoi(param);					
			get_from_flash_P(PSTR("MESTIME: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->measure_time, DEC);
			_analog->set_measure_time(_config->measure_time);	
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
				if (sync_EEPROM(HTTP_STRING, param, strlen(param)+1))
					get_from_flash_P(PSTR("URL W"), _buff);
				else
					get_from_flash_P(PSTR("URL OK"), _buff);
				Serial.println(_buff);
			}
		} else if (line[5] == 'S') { // HTTP_STATUS_TIME
			_config->http_status_time = atol(param)*60;
			get_from_flash_P(PSTR("HST: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->http_status_time, DEC);
		} else if (line[5] == 'U' && line[6] == 'P') { // HTTP_UPLOAD_TIME
			_config->http_upload_time = atol(param)*60;
			get_from_flash_P(PSTR("HUT: "), _buff);
			Serial.print(_buff);
			Serial.println(_config->http_upload_time, DEC);
		}
	} else if (strncmp_P(line, PSTR("GP"), 2) == 0) { // GPRS params
		if (line[5] == 'A') { // GPRS_APN
			param = fforward(param);
			if (param != NULL) {
				if (sync_EEPROM(APN_STRING, param, strlen(param)+1))
					get_from_flash_P(PSTR("APN W"), _buff);
				else
					get_from_flash_P(PSTR("APN OK"), _buff);
				Serial.println(_buff);
			}
		} else if (line[5] == 'U') { // GPRS_USER
		} else if (line[5] == 'P') { // GPRS_PASS
		}
	}
        return 0;
}

uint8_t DLConfig::load() {
	unsigned long filesize = 0;
	if (_sd->is_available() > 0) {
		filesize = _sd->open(CONFIG, O_READ);
		_sd->rewind(CONFIG);
		int rv = 0;
		do {
			rv = _sd->read(CONFIG, _buff, _buff_size, '\n');
			if (_buff[0] != '#' && _buff[0] != 0 && _buff[0] != '\n') {
				log_process_callback(_buff, _buff_size);
			}
		} while (rv >= 0);
		_sd->close(CONFIG);
		return 1;
	} else {
		_sd->init();
		return 0;
	}
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
		load_EEPROM(addr+(i*sizeof(v)), (char *)&v, sizeof(v));
		Serial.print("LFC: ");
		Serial.println(v);
		if (!saved)
			_sd->set_files_count(i, v);
		else	
			_sd->set_saved_count(i, v);
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
		if (!saved)
			v = _sd->get_files_count(i); // Get the current pos
		else
			v = _sd->get_saved_count(i);
		Serial.print("SFC: ");
		Serial.println(v);
		sync_EEPROM(addr+(i*sizeof(v)), (char *)&v, sizeof(v));
	}
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


