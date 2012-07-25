#include <Arduino.h>
#include "DLMeasure.h"

#define MASK(v,p) (v & (0x1 << p))

volatile char previous_portvals = 0;
volatile double _vals[NUM_IO] = { 0 };
volatile double _std_dev[NUM_IO] = { 0 };
volatile double _mins[NUM_IO] = { 9999.0 };
volatile double _maxs[NUM_IO] = { 0 };
volatile uint32_t _cnt_vals[NUM_DIGITAL] = { 0 };
volatile uint8_t _AOD[NUM_IO] = { IO_OFF };
volatile uint32_t isr_cnt = 0;
volatile char got_event = 0;

ISR(DIGITAL_ISR_VECT) {
	unsigned char portvals, i;
	unsigned char changed;
	portvals = DIGITAL_PORT;
	changed = portvals ^ previous_portvals;

	for(i = 0; i < NUM_DIGITAL; i++) {
		if (_AOD[DIGITAL_OFFSET+i] == IO_EVENT) { 
			if (!MASK(previous_portvals, 7-i) && MASK(portvals, 7-i)) { // Rising edge
				got_event = 1;
				_vals[DIGITAL_OFFSET+i] = 1;
				_std_dev[DIGITAL_OFFSET+i] = (uint32_t)millis();
			} 
			else if (MASK(previous_portvals, 7-i) && !MASK(portvals, 7-i)) { // Falling edge
				got_event = 1;
				_vals[DIGITAL_OFFSET+i] = 0;
				_std_dev[DIGITAL_OFFSET+i] = (uint32_t)millis();
			}
		} else if (_AOD[DIGITAL_OFFSET+i] == IO_COUNTER && MASK(changed, 7-i) && MASK(portvals, 7-i)) {
				_cnt_vals[i]++;
		}
	}
	previous_portvals = portvals;
	isr_cnt++;
}

DLMeasure::DLMeasure()
{
	_sum_cnt = 0;
	_measure_time = 60;
	_int_ptr = NULL;
	_count_start = 0;
}

void DLMeasure::init() {
	_count_start = 0;
	for(uint8_t i=ANALOG_OFFSET;i<NUM_IO;i++) {
		_AOD[i] = IO_OFF;
		pinMode(num2pin_mapping[i], INPUT);
		digitalWrite(num2pin_mapping[i], HIGH); // Turn on internal pullup
	}
	// Enable PCINT for digital ports
	DIGITAL_PCMSK = DIGITAL_PCMSK_VAL; // Only enable the ports in use
	PCICR |= (1 << DIGITAL_PCIE);
	_smeasure = 0;
	enable();
}

void DLMeasure::set_measure_time(uint16_t measure_time) {
	_measure_time = measure_time;
}

void DLMeasure::enable() {
	digitalWrite(_en, LOW);
}

void DLMeasure::disable(){
	digitalWrite(_en, HIGH);
}

void DLMeasure::pwr_on() {
	digitalWrite(EXT_PWR_PIN, HIGH);
}

void DLMeasure::pwr_off() {
	digitalWrite(EXT_PWR_PIN, LOW);
}

uint16_t DLMeasure::read(uint8_t pin){
	uint16_t ret = 0;
	uint8_t r = 0;
	if (_AOD[pin] == IO_OFF)
		return 0;

	if (_AOD[pin] == IO_ANALOG) {
		ret = analogRead(num2pin_mapping[pin]); //map(analogRead(_inp), 0, 1023, 0, _bandgap);
		_vals[pin] += (double)ret;
		_std_dev[pin] += (double)ret * (double)ret;
		if ((double)ret < _mins[pin])
			_mins[pin] = (double)ret;
		if ((double)ret > _maxs[pin])
			_maxs[pin] = (double)ret;	
	}
	else if (_AOD[pin] == IO_DIGITAL) {
		ret = digitalRead(num2pin_mapping[pin]);
		_vals[pin] = (double)ret;
	}
	return ret;
}

uint8_t DLMeasure::read_all(uint8_t itr){
	uint16_t v = 0;
	double tmpv = 0.0;
	if (_smeasure == 0)
		_smeasure = millis();

	for(uint8_t i = ANALOG_OFFSET; i < NUM_IO; i++) {
		if (_AOD[i] == IO_COUNTER) {
			tmpv = (double)_cnt_vals[i-DIGITAL_OFFSET];
			_cnt_vals[i-DIGITAL_OFFSET] = 0;
			_vals[i] += tmpv;
			_std_dev[i] += tmpv*tmpv;
		} else {
			read(i);
		}
	}
	_sum_cnt++;
	return _sum_cnt;		 
}

uint8_t DLMeasure::read_all() {
	return read_all(0);
}

uint8_t DLMeasure::get_all() {
	uint8_t rdy = 0;
	double dtime = ((double)millis() - (double)_smeasure) / 1000.0;
	
	for(uint8_t i=ANALOG_OFFSET;i<NUM_IO;i++) {
			if (_AOD[i] == IO_ANALOG) { // Only process analog
				_std_dev[i] = (double)(sqrt((_sum_cnt*_std_dev[i]) - (_vals[i]*_vals[i])) / _sum_cnt); // Rolling stddev
				_vals[i] = (double)(_vals[i] / _sum_cnt); // Mean
			} else if (_AOD[i] == IO_COUNTER) {
				_std_dev[i] = (double)(sqrt((dtime*_std_dev[i]) - (_vals[i]*_vals[i])) / dtime);
				_vals[i] = (double)(_vals[i] / dtime);
			}		
	}
	rdy = 1;
	_smeasure = 0;	
	return rdy;
}

void DLMeasure::reset() {
	for(uint8_t i=ANALOG_OFFSET;i<NUM_IO;i++) {
		if (_AOD[i] == IO_DIGITAL || _AOD[i] == IO_ANALOG) {
			_vals[i] = 0;
			_std_dev[i] = 0;
			_maxs[i] = 0;
			_mins[i] = 1024;
		} else if (_AOD[i] == IO_COUNTER) {
			_std_dev[i] = 0;
			_vals[i] = 0;
		}
	}
	_sum_cnt = 0;
	_smeasure = 0;	
}

void DLMeasure::set_int_fun(INT_callback fun) {
	_int_ptr = fun;
}

void DLMeasure::set_pin(uint8_t pin, uint8_t doa){
	if ((pin == 0 || pin == 1) && doa == IO_EVENT && _int_ptr) {
//		attachInterrupt(pin, (INT_callback)_int_ptr, CHANGE);
	}
	if (doa != IO_OFF) {
		digitalWrite(num2pin_mapping[pin], LOW); // Turn off interal pullup
	}
	_AOD[pin] = doa;
}

uint8_t DLMeasure::get_pin(uint8_t pin) {
	return _AOD[pin];
}

void DLMeasure::time_log_line(char *line) {
	char tmpbuff[13];
	uint32_t n = 0;
	*line = '\0';
	strcat(line, "T");
	fmtUnsigned(now(), tmpbuff, 12);
	strcat(line, tmpbuff);
	strcat(line, " V");
	fmtUnsigned(get_supply_voltage(), tmpbuff, 12);
	strcat(line, tmpbuff);
	for(uint8_t i=ANALOG_OFFSET;i<NUM_IO;i++) {
		if (_AOD[i] != IO_OFF)
			strcat(line, " ");

		if (_AOD[i] == IO_ANALOG) {
			strcat(line, "a");
			fmtUnsigned(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_vals[i], 2, tmpbuff, 12);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_std_dev[i], 2, tmpbuff, 12);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_mins[i], 2, tmpbuff, 12);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_maxs[i], 2, tmpbuff, 12);
			strcat(line, tmpbuff);	
		} else if (_AOD[i] == IO_DIGITAL) {
			strcat(line, "d");
			fmtUnsigned(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtUnsigned(_vals[i], tmpbuff, 12);
			strcat(line, tmpbuff);
		} else if (_AOD[i] == IO_COUNTER) {
			strcat(line, "c");
			fmtUnsigned(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_vals[i], 2, tmpbuff, 12);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtDouble((double)_std_dev[i], 2, tmpbuff, 12);
			strcat(line, tmpbuff);
		}
	}
	strcat(line, "\r\n");
}

void DLMeasure::event_log_line(char *line) {
	uint8_t i=0;
	char tmpbuff[13];
	*line = '\0';
	strcat(line, "E");
	fmtUnsigned(now(), tmpbuff, 12);
	//ltoa(now(), tmpbuff, 16);
	strcat(line, tmpbuff);
	strcat(line, " ");
	for(i=DIGITAL_OFFSET;i<NUM_IO;i++) {
		if (_AOD[i] == IO_EVENT) {
			if (i != 0)
				strcat(line, " ");
			fmtUnsigned(i, tmpbuff, 10);
			//itoa(i, tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			fmtUnsigned(_vals[i], tmpbuff, 12);
			//itoa(_vals[i], tmpbuff, 10);
			strcat(line, tmpbuff);
			strcat(line, ":");
			//ltoa((unsigned long)_std_dev[i], tmpbuff,10);
			fmtUnsigned(_std_dev[i], tmpbuff, 12);
			strcat(line, tmpbuff);
		}
	}
	strcat(line, "\r\n");
}

float DLMeasure::get_voltage(uint8_t pin) {
	float r = (_vals[pin] / 1023.0) * VREF;
	return r;
}

void DLMeasure::debug(uint8_t v) {
	_DEBUG = v;
}

char DLMeasure::check_event() {
	return got_event;
}

void DLMeasure::reset_event() {
	got_event = 0;
}
