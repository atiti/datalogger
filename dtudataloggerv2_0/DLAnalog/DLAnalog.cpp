#include "WProgram.h"
#include "DLAnalog.h"

#define MASK(v,p) (v & (0x1 << p))

DLAnalog::DLAnalog(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t en, uint8_t inp, bool pullup)
{
	pinMode(s0, OUTPUT);
	pinMode(s1, OUTPUT);
	pinMode(s2, OUTPUT);
	pinMode(s3, OUTPUT);
	pinMode(en, OUTPUT);
	pinMode(inp, INPUT);
	digitalWrite(inp, pullup);	
	_s[0] = s0;
	_s[1] = s1;
	_s[2] = s2;
	_s[3] = s3;
	_en = en;
	_inp = inp;
	_pullup = pullup;
	_sum_cnt = 0;
	memset(_AOD, OFF, sizeof(_AOD));
}

void DLAnalog::init(volatile uint16_t *vals, volatile uint32_t *std_dev) {
	_vals = vals;
	_std_dev = std_dev;
}

void DLAnalog::enable() {
	digitalWrite(_en, LOW);
}

void DLAnalog::disable(){
	digitalWrite(_en, HIGH);
}

uint16_t DLAnalog::read(uint8_t pin){
	uint16_t ret = 0;
	uint8_t r = 0;
	if (_AOD[pin] == OFF)
		return 0;

	for(uint8_t i=0;i<4;i++) {  // Set up the address for the read
		if ((pin & (0x1 << i)) > 0)
			digitalWrite(_s[i], HIGH);
		else
			digitalWrite(_s[i], LOW);
	}
	// Since the analog mux is fast enough, we can already read out
	// the value without delay
	if (_AOD[pin] == ANALOG)
		ret = analogRead(_inp);
	else if (_AOD[pin] == DIGITAL || _AOD[pin] == EVENT)
		ret = digitalRead(_inp);
	else if (_AOD[pin] == COUNTER) {
		r = digitalRead(_inp);
		if ((!r && MASK(_dvals,pin))) {
			if (!_std_dev[pin]) {
				_std_dev[pin] = millis();
				_vals[pin] = 1;
			} else {
				_vals[pin]++;
			}
		}
		if  (!r) {	
			_dvals = _dvals & ~(0x1 << pin); // Set pin value to 0
		} else {
			_dvals = _dvals | (0x1 << pin); // Set pin value to 1
		}
	} else if (_AOD[pin] == EVENT) {
		_vals[pin] = digitalRead(_inp);
		_std_dev[pin] = millis();
	}
	return ret;
}

uint8_t DLAnalog::read_all(uint8_t itr){
	uint16_t v = 0;
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] != EVENT && _AOD[i] != OFF && !itr) { // Only read ports which arent done by interrupt
			if (_AOD[i] == ANALOG) { // Analog summing
				v = read(i);
				_vals[i] += v;
				_std_dev[i] += pow(v, 2);
			}
			else if (_AOD[i] == DIGITAL) // Digital just read directly
				_vals[i] = read(i);
			else if (_AOD[i] == COUNTER)
				read(i);
		}
		else if (itr && _AOD[i] == EVENT) {
			read(i);
		}
	}
	if (!itr)
		_sum_cnt++;
	return _sum_cnt;		 
}

uint8_t DLAnalog::read_all() {
	return read_all(0);
}

uint8_t DLAnalog::get_all() {
	uint8_t rdy = 0;
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] == ANALOG) { // Only process analog
			_std_dev[i] = sqrt((_sum_cnt*_std_dev[i]) - pow(_vals[i], 2)) / _sum_cnt; // Rolling stddev
			_vals[i] = _vals[i] / _sum_cnt; // Mean
		} else if (_AOD[i] == COUNTER) {
			if ((millis() - _std_dev[i]) > 60000 && _std_dev[i] != 0) {
				_std_dev[i] = 0;
				rdy = 1;
			}		
		}
	}
	return rdy;
}

void DLAnalog::reset() {
	for(uint8_t i=0;i<16;i++)
		if (_AOD[i] == DIGITAL || _AOD[i] == ANALOG) {
			_vals[i] = 0;
			_std_dev[i] = 0;
		} else if (_AOD[i] == COUNTER) {
			if (_std_dev[i] == 0)
				_vals[i] = 0;
		}
	_sum_cnt = 0;
}

void DLAnalog::set_pin(uint8_t pin, uint8_t doa){
	_AOD[pin] = doa;
}

uint8_t DLAnalog::get_pin(uint8_t pin) {
	return _AOD[pin];
}

void DLAnalog::time_log_line(char *line) {
	char tmpbuff[11];
	uint16_t first = 1;
	strcpy(line, "T");
	ltoa(now(), tmpbuff, 16);
	strcat(line, tmpbuff);
	strcat(line, "\t");
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] == ANALOG) {
			if (first == 0)
				strcat(line, "\t");
			else 
				first = 0;
			strcat(line, "a");
			itoa(i, tmpbuff, 10); // hexadecimal
			strcat(line, tmpbuff);
			strcat(line, ":");
			itoa(_vals[i], tmpbuff, 10); // hexadecimal
			strcat(line, tmpbuff);
			strcat(line, "\ts");
			itoa(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			itoa((uint32_t)_std_dev[i], tmpbuff, 10);
			strcat(line, tmpbuff);	
		} else if (_AOD[i] == DIGITAL) {
			if (first == 0)
				strcat(line, "\t");
			else
				first = 0;
			strcat(line, "d");
			itoa(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			itoa(_vals[i], tmpbuff, 10);
			strcat(line, tmpbuff);
		} else if (_AOD[i] == COUNTER && _std_dev[i] == 0) {
			if (first == 0)
				strcat(line, "\t");
			else
				first = 0;
			strcat(line, "c");
			itoa(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			itoa(_vals[i], tmpbuff,10);
			strcat(line, tmpbuff);
		}
	}
	strcat(line, "\n");
}

void DLAnalog::event_log_line(char *line) {
	char tmpbuff[11];
	strcpy(line, "E");
	ltoa(now(), tmpbuff, 16);
	strcat(line, tmpbuff);
	strcat(line, "\t");
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] == EVENT) {
			if (i != 0)
				strcat(line, "\t");
			itoa(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			itoa(_vals[i], tmpbuff, 10);
			strcat(line, tmpbuff);
		}
	}
	strcat(line, "\n");
}

float DLAnalog::get_voltage(uint8_t pin) {
	float r = (_vals[pin] / 1023.0) * VREF;
	return r;
}

void DLAnalog::debug(uint8_t v) {
	_DEBUG = v;
}
