#include <Arduino.h>
#include "DLMeasure.h"

#define MASK(v,p) (v & (0x1 << p))

char previous_portvals = 0;
volatile double _vals[NUM_IO] = { 0 };
volatile double _std_dev[NUM_IO] = { 0 };
volatile double _mins[NUM_IO] = { 9999.0 };
volatile double _maxs[NUM_IO] = { 0 };
volatile uint8_t _AOD[NUM_IO] = { IO_OFF };

/*
ISR(DIGITAL_ISR_VECT) {
	char portvals, i;
	portvals = DIGITAL_PORT;
	for(i = 0; i < NUM_DIGITAL; i++) {
		if (_AOD[DIGITAL_OFFSET+i] == IO_EVENT) { 
			if (!MASK(previous_portvals, i) && MASK(portvals, i)) { // Rising edge
				_vals[DIGITAL_OFFSET+i] = 1;
				_std_dev[DIGITAL_OFFSET+i] = millis();
			} 
			else if (MASK(previous_portvals, i) && !MASK(portvals, i)) { // Falling edge
				_vals[DIGITAL_OFFSET+i] = 0;
				_std_dev[DIGITAL_OFFSET+i] = millis();
			}
		} else if (_AOD[DIGITAL_OFFSET+i] == IO_COUNTER) {
			if (!MASK(previous_portvals, i) && MASK(portvals, i)) { // Only deal with rising edge
				_vals[DIGITAL_OFFSET+i]++;
			}
		}
	}
}
*/
DLMeasure::DLMeasure()
{
	_sum_cnt = 0;
	_measure_time = 60;
	_int_ptr = NULL;
	_count_start = 0;
}

void DLMeasure::init() {
	_count_start = 0;
	for(uint8_t i=ANALOG_OFFSET;i<(ANALOG_OFFSET+NUM_ANALOG);i++) {
		_AOD[i] = IO_OFF;
		pinMode(num2pin_mapping[i], INPUT);
	}
	// Enable PCINT for digital ports
	DIGITAL_PCMSK = DIGITAL_PCMSK_VAL; // Only enable the ports in use
	PCICR |= (1 << DIGITAL_PCIE);

	enable();
//	get_bandgap();
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
	delay(50);
	//get_bandgap();
}

void DLMeasure::pwr_off() {
	digitalWrite(EXT_PWR_PIN, LOW);
	delay(50);
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
	for(uint8_t i = ANALOG_OFFSET; i < (ANALOG_OFFSET+NUM_ANALOG); i++) {
		read(i);
	}
	_sum_cnt++;
	return _sum_cnt;		 
}

uint8_t DLMeasure::read_all() {
	return read_all(0);
}

uint8_t DLMeasure::get_all() {
	uint8_t rdy = 0;
	for(uint8_t i=ANALOG_OFFSET;i<(ANALOG_OFFSET+NUM_ANALOG);i++) {
			if (_AOD[i] == IO_ANALOG) { // Only process analog
				_std_dev[i] = (double)(sqrt((_sum_cnt*_std_dev[i]) - (_vals[i]*_vals[i])) / _sum_cnt); // Rolling stddev
				_vals[i] = (double)(_vals[i] / _sum_cnt); // Mean
			} else if (_AOD[i] == IO_COUNTER) {
				_std_dev[i] = (double)(sqrt((_sum_cnt*_std_dev[i]) - (_vals[i]*_vals[i])) / _sum_cnt);
				_vals[i] = (double)((_vals[i] / _sum_cnt) * MEASURE_MULTIPLIER);
//				_vals[i] = _vals[i] * MEASURE_MULTIPLIER;
//				_std_dev[i] = (double)(sqrt(_std_dev[i] / (_sum_cnt-1)));
			}		
	}
	_smeasure = 0;
	rdy = 1;
	
	return rdy;
}

void DLMeasure::reset() {
	for(uint8_t i=ANALOG_OFFSET;i<(ANALOG_OFFSET+NUM_ANALOG);i++) {
		if (_AOD[i] == IO_DIGITAL || _AOD[i] == IO_ANALOG) {
			_vals[i] = 0;
			_std_dev[i] = 0;
			_maxs[i] = 0;
			_mins[i] = 1024;
		} else if (_AOD[i] == IO_COUNTER && _smeasure == 0) {
			_std_dev[i] = 0;
			_vals[i] = 0;
		}
	}
	if (_smeasure == 0) {
		_sum_cnt = 0;
		_count_start = 0;
	}
	//get_bandgap();
}

void DLMeasure::set_int_fun(INT_callback fun) {
	_int_ptr = fun;
}

void DLMeasure::set_pin(uint8_t pin, uint8_t doa){
	if ((pin == 0 || pin == 1) && doa == IO_EVENT && _int_ptr) {
//		attachInterrupt(pin, (INT_callback)_int_ptr, CHANGE);
	}
	_AOD[pin] = doa;
}

uint8_t DLMeasure::get_pin(uint8_t pin) {
	return _AOD[pin];
}

void DLMeasure::time_log_line(char *line) {
	char tmpbuff[13];
	uint32_t n = 0;
	strcpy(line, "T");
	fmtUnsigned(now(), tmpbuff, 12);
	strcat(line, tmpbuff);
	strcat(line, " V");
	fmtUnsigned(_bandgap, tmpbuff, 12);
	strcat(line, tmpbuff);
	for(uint8_t i=ANALOG_OFFSET;i<(ANALOG_OFFSET+NUM_ANALOG);i++) {
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
	char tmpbuff[13];
	strcpy(line, "E");
	fmtUnsigned(now(), tmpbuff, 12);
	//ltoa(now(), tmpbuff, 16);
	strcat(line, tmpbuff);
	strcat(line, " ");
	for(uint8_t i=ANALOG_OFFSET;i<(ANALOG_OFFSET+NUM_ANALOG);i++) {
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

int DLMeasure::get_bandgap(void)
{
        const long InternalReferenceVoltage = 1100L;  // Adust this value to your specific internal BG voltage x1000
	for(uint8_t i=0;i<3;i++) { // Read out 3 times for it to stabilize

        	// REFS1 REFS0          --> 0 1, AVcc internal ref.
        	// MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)
       		ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
        	// Start a conversion  
        	ADCSRA |= _BV( ADSC );
        	// Wait for it to complete
        	while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
        	// Scale the value
        	_bandgap = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L;
	}
  
      return _bandgap;
}

void DLMeasure::debug(uint8_t v) {
	_DEBUG = v;
}
