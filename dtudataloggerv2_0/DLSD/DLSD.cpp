#include "WProgram.h"
#include "DLSD.h"

prog_char sd_filename_0[] PROGMEM = "DATALOG.TXT";
prog_char sd_filename_1[] PROGMEM = "SYSTEM.TXT";
prog_char sd_filename_2[] PROGMEM = "CONFIG.TXT";
prog_char sd_filename_3[] PROGMEM = "EVENT.TXT";

PROGMEM const char *sd_filename_table[] = {sd_filename_0, sd_filename_1,
					   sd_filename_2, sd_filename_3};

DLSD::DLSD(bool fullspeed, int CS)
{
	_CS = CS;
	_fullspeed = fullspeed;
}

int8_t DLSD::init() {
	uint8_t ret = 0;
	if (_fullspeed)
		ret = _card.init(SPI_FULL_SPEED, _CS);
	else
		ret = _card.init(SPI_HALF_SPEED, _CS);
	if (!ret) // Failed to initialize card
		return -1;
	if (!_vol.init(&_card)) // Failed to initialize volume
		return -2;
	if (!_root.openRoot(&_vol))  // Open the root folder
		return -3;
	
	return 1;
}

void DLSD::debug(int v){
	_DEBUG = v;
}

unsigned long DLSD::open(uint8_t n, uint8_t flags) {
	uint8_t ret;
	unsigned long fsize = 0;
	digitalWrite(_CS, LOW);
	for(int j=0;j<4;j++) {
		if (_files_open[j] == true && j != n) {
			close(n);
		}
	}
	if (_files_open[n] == false) {
		get_from_flash(&(sd_filename_table[n]), _filename);
		ret = _files[n].open(&_root, _filename, flags);
		_files_open[n] = true;
		fsize = _files[n].fileSize();
	} else {
		fsize = _files[n].fileSize();
	}
	return fsize;
}

bool DLSD::close(uint8_t n) {
	_files[n].sync();
	if (!_files[n].close())
		return false;
	_files_open[n] = false;
	digitalWrite(_CS, HIGH);
	return true;
}

bool DLSD::write(uint8_t n, char *ptr) {
	_files[n].writeError = false;
	_files[n].print(ptr);
	_files[n].sync();
	return _files[n].writeError;
}
		
bool DLSD::write(uint8_t n, float a) {
	_files[n].writeError = false;
	_files[n].print(a);
	_files[n].sync();
	return _files[n].writeError;
}

bool DLSD::write(uint8_t n, unsigned short a) {
	_files[n].writeError = false;
	_files[n].print(a);
	_files[n].sync();
	return _files[n].writeError;
}

bool DLSD::write(uint8_t n, int a) {
	_files[n].writeError = false;
	_files[n].print(a);
	_files[n].sync();
	return _files[n].writeError;
}

bool DLSD::write(uint8_t n, unsigned long a) {
	_files[n].writeError = false;
	_files[n].print(a);
	_files[n].sync();
	return _files[n].writeError;
}

void DLSD::read(uint8_t n, char *ptr, int len) {
	_files[n].read(ptr, len);
}

void DLSD::read(uint8_t n, char *ptr, int len, char t) {
	int c = 0;
	while (c < len) {
		ptr[c] = _files[n].read();
		if (ptr[c] < 0 || ptr[c] == t) break;
		c++;
	}
}

void DLSD::rewind(uint8_t n) {
	_files[n].rewind();
}

bool DLSD::seek(uint8_t n, uint32_t pos) {
	_files[n].seekSet(pos);
}
                
bool DLSD::seekend(uint8_t n) {
	_files[n].seekEnd();
}


