#include "WProgram.h"
#include "DLConfig.h"

DLConfig::DLConfig()
{
	_config = NULL;
	_sd = NULL;
}

void DLConfig::init(Config *config, DLSD *sd, DLAnalog *analog, char *buff, int len) {
	_config = config;
	_sd = sd;
	_analog = analog;
	_buff = buff;
	_buff_size = len-1;
}

int DLConfig::log_process_callback(char *line, int len) {
        char *param, *ptr;
        //. Here we extract the name and value combination by finding the equal sign
        ptr = strstr(line, "=");
        param = ptr + 1;
        *ptr = 0;

        Serial.println(line);
        if (strncmp_P(line, PSTR("PORT"), 4) == 0) {
                ptr = line+10;
                Serial.println(atoi(ptr));
                if (strncmp_P(line, PSTR("PORT_MOD"), 8) == 0) {

                } else if (strncmp_P(line, PSTR("PORT_TRIG"), 9) == 0) {

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
