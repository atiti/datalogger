#include "WProgram.h"
#include "DLAnalog.h"

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
	for(int i=0;i<sizeof(_AOD);i++) {
		_AOD[i] = 0;
	}
}

void DLAnalog::enable() {
	digitalWrite(_en, LOW);
}

void DLAnalog::disable(){
	digitalWrite(_en, HIGH);
}

int DLAnalog::read(int pin){
	int ret = 0;
	uint8_t mask = 0x1;
	for(int i=0;i<4;i++) {  // Set up the address for the read
		if ((pin & (mask << i)) > 0)
			digitalWrite(_s[i], HIGH);
		else
			digitalWrite(s_[i], LOW);
	}
	// Since the analog mux is fast enough, we can already read out
	// the value without delay
	if (_AOD[pin])
		ret = analogRead(_inp);
	else
		ret = digitalRead(_inp);
	return ret;
}

float DLAnalog::readAll(unsigned short *vals){
	float starttime = 0;
	if (_DEBUG)	
		starttime = millis();
	for(int i=0;i<16;i++)
		vals[i] = read(i);
	if (_DEBUG)
		return (millis()-starttime);
	return 0;		 
}

void DLAnalog::setPin(int pin, int doa){
	_AOD[pin] = doa;
}

void debug(int v) {
	_DEBUG = v;
}
