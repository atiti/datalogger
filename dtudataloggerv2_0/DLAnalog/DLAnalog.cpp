#include "WProgram.h"
#include "DLAnalog.h"

#define MASK(v,p) (v & (0x1 << p))


DLAnalog::DLAnalog(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t en, uint8_t inp, bool pullup)
{
	pinMode(EXT_PWR_PIN, OUTPUT);
	digitalWrite(EXT_PWR_PIN, LOW);
	pinMode(s0, OUTPUT);
	pinMode(s1, OUTPUT);
	pinMode(s2, OUTPUT);
	pinMode(s3, OUTPUT);
	pinMode(en, OUTPUT);
	pinMode(INT0, INPUT);
	pinMode(INT1, INPUT);
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
	_measure_time = 60;
	_int_ptr = NULL;
	_count_start = 0;
	memset(_AOD, IO_OFF, sizeof(_AOD));
}

void DLAnalog::init(double *vals, double *std_dev, uint8_t *count_values, uint16_t measure_time) {
	_vals = vals;
	_std_dev = std_dev;
	_measure_time = measure_time;
	_count_vals = count_values;
	_count_start = 0;
	enable();
}

void DLAnalog::set_measure_time(uint16_t measure_time) {
//	_measure_time = measure_time;
}

void DLAnalog::enable() {
	digitalWrite(_en, LOW);
}

void DLAnalog::disable(){
	digitalWrite(_en, HIGH);
}

void DLAnalog::pwr_on() {
	digitalWrite(EXT_PWR_PIN, HIGH);
	delay(50);
}

void DLAnalog::pwr_off() {
	digitalWrite(EXT_PWR_PIN, LOW);
	delay(50);
}

uint16_t DLAnalog::read(uint8_t pin){
	uint16_t ret = 0;
	uint8_t r = 0;
	if (_AOD[pin] == IO_OFF)
		return 0;

	if (!_smeasure)
		_smeasure = now();

	for(uint8_t i=0;i<4;i++) {  // Set up the address for the read
		if ((pin & (0x1 << i)) > 0)
			digitalWrite(_s[i], HIGH);
		else
			digitalWrite(_s[i], LOW);
	}
	// Since the analog mux is fast enough, we can already read out
	// the value without delay
	if (_AOD[pin] == IO_ANALOG) {
		ret = analogRead(_inp);
		_vals[pin] += (double)ret;
		_std_dev[pin] += (double)ret * (double)ret;
	}
	else if (_AOD[pin] == IO_DIGITAL) {
		ret = digitalRead(_inp);
		_vals[pin] = (double)ret;
	}
	else if (_AOD[pin] == IO_COUNTER) {
		r = digitalRead(_inp);
		if (_count_start == 0) {
			_count_vals[pin] = 0;
			_count_start = millis();
		}

		if ((!r && MASK(_dvals,pin))) {
			_count_vals[pin]++;
		}
		if  (!r) {	
			_dvals = _dvals & ~(0x1 << pin); // Set pin value to 0
		} else {
			_dvals = _dvals | (0x1 << pin); // Set pin value to 1
		}
	} else if (_AOD[pin] == IO_EVENT) {
		ret = analogRead(_inp);
		_vals[pin] = ret;
		_std_dev[pin] = millis();
	}
	return ret;
}

uint8_t DLAnalog::read_all(uint8_t itr){
	uint16_t v = 0;
	double tmpv = 0.0;
	for(uint8_t i = 0; i < 16; i++) {
		if (_AOD[i] == IO_COUNTER || _AOD[i] == IO_EVENT)
			read(i);
	}
	if ((millis()-_count_start) >= MEASURE_RATE && _count_start != 0) {
		for(uint8_t i=0;i<16;i++) {
			if (_AOD[i] == IO_COUNTER) {
				_vals[i] += _count_vals[i];
				_std_dev[i] += _count_vals[i] * _count_vals[i];
				//Serial.print(_sum_cnt, DEC);
				//Serial.print(" ");
				//Serial.print(_count_vals[i], DEC);
				//Serial.print(" ");
				//Serial.print(_vals[i], DEC);
				//Serial.print(" ");
				//Serial.println(_std_dev[i], DEC);
				_count_vals[i] = 0;
			} else {
				tmpv = read(i);
				/*if (i == 15) {
					Serial.print(_sum_cnt, DEC);
					Serial.print(" ");
					Serial.print(tmpv, DEC);
					Serial.print(" ");
					Serial.print(_vals[i], DEC);
					Serial.print(" ");
					Serial.println(_std_dev[i], DEC);
				}*/	
			}
		}
		_count_start = 0;
		_sum_cnt++;
	}	
	if (_smeasure == 0)
		_smeasure = now();
	return _sum_cnt;		 
}

uint8_t DLAnalog::read_all() {
	return read_all(0);
}

uint8_t DLAnalog::get_all() {
	uint8_t rdy = 0;
	if (((now() - _smeasure) >= _measure_time) && _smeasure != 0) {
		for(uint8_t i=0;i<16;i++) {
			if (_AOD[i] == IO_ANALOG) { // Only process analog
				_std_dev[i] = (double)(sqrt((_sum_cnt*_std_dev[i]) - (_vals[i]*_vals[i])) / _sum_cnt); // Rolling stddev
				_vals[i] = (double)(_vals[i] / _sum_cnt); // Mean
			} else if (_AOD[i] == IO_COUNTER) {
				_std_dev[i] = (double)(sqrt((_sum_cnt*_std_dev[i]) - (_vals[i]*_vals[i])) / _sum_cnt);
				_vals[i] = (double)((_vals[i] / _sum_cnt) * MEASURE_MULTIPLIER);
//				_vals[i] = _vals[i] * MEASURE_MULTIPLIER;
//				_std_dev[i] = (double)(sqrt(_std_dev[i] / (_sum_cnt-1)));
				_count_vals[i] = 0;
			}		
		}
		_smeasure = 0;
		rdy = 1;
	}
	
	return rdy;
}

void DLAnalog::reset() {
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] == IO_DIGITAL || _AOD[i] == IO_ANALOG) {
			_vals[i] = 0;
			_std_dev[i] = 0;
		} else if (_AOD[i] == IO_COUNTER && _smeasure == 0) {
			_std_dev[i] = 0;
			_vals[i] = 0;
		}
	}
	if (_smeasure == 0) {
		_sum_cnt = 0;
		_count_start = 0;
	}
}

void DLAnalog::set_int_fun(INT_callback fun) {
	_int_ptr = fun;
}

void DLAnalog::set_pin(uint8_t pin, uint8_t doa){
	if ((pin == 0 || pin == 1) && doa == IO_EVENT && _int_ptr) {
		attachInterrupt(pin, (INT_callback)_int_ptr, CHANGE);
	}
	_AOD[pin] = doa;
}

uint8_t DLAnalog::get_pin(uint8_t pin) {
	return _AOD[pin];
}

void DLAnalog::time_log_line(char *line) {
	char tmpbuff[11];
	uint32_t n = 0;
	strcpy(line, "T");
	fmtUnsigned(now(), tmpbuff, 12);
	strcat(line, tmpbuff);
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] != IO_OFF)
			strcat(line, "\t");

		if (_AOD[i] == IO_ANALOG) {
			strcat(line, "a");
			fmtUnsigned(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_vals[i], 2, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_std_dev[i], 2, tmpbuff, 10);
			strcat(line, tmpbuff);	
		} else if (_AOD[i] == IO_DIGITAL) {
			strcat(line, "d");
			fmtUnsigned(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtUnsigned(_vals[i], tmpbuff, 10);
			strcat(line, tmpbuff);
		} else if (_AOD[i] == IO_COUNTER) {
			strcat(line, "c");
			fmtUnsigned(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_vals[i], 2, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_std_dev[i], 2, tmpbuff, 10);
			strcat(line, tmpbuff);
		}
	}
	strcat(line, "\n");
}

void DLAnalog::event_log_line(char *line) {
	char tmpbuff[11];
	strcpy(line, "E");
	fmtUnsigned(now(), tmpbuff, 12);
	//ltoa(now(), tmpbuff, 16);
	strcat(line, tmpbuff);
	strcat(line, "\t");
	for(uint8_t i=0;i<16;i++) {
		if (_AOD[i] == IO_EVENT) {
			if (i != 0)
				strcat(line, "\t");
			fmtUnsigned(i, tmpbuff, 10);
			//itoa(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtUnsigned(_vals[i], tmpbuff, 10);
			//itoa(_vals[i], tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			//ltoa((unsigned long)_std_dev[i], tmpbuff,10);
			fmtUnsigned(_std_dev[i], tmpbuff, 10);
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
