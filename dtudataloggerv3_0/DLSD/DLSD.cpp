#include <Arduino.h>
#include "DLSD.h"

prog_char sd_filename_0[] PROGMEM = "CONFIG";
//prog_char sd_filename_1[] PROGMEM = "SYS00000";
prog_char sd_filename_1[] PROGMEM = "DAT00000";
//prog_char sd_filename_3[] PROGMEM = "EVE00000";

PROGMEM const char *sd_filename_table[] = {sd_filename_0, sd_filename_1 };
                                           //sd_filename_2, sd_filename_3};

prog_char sd_filename_ext[] PROGMEM = ".DAT";

DLSD::DLSD(char fullspeed, uint8_t CS)
{
	_CS = CS;
	_inited = 0;
	_fullspeed = fullspeed;
	for(uint8_t i = 0; i < NUM_FILES; i++)
		_files_count[i] = 0;
}

int8_t DLSD::init() {
	uint8_t ret = 0;
//	if (_fullspeed)
//		ret = _card.init(SPI_FULL_SPEED, _CS);
//	else
	ret = _card.init(_fullspeed, _CS);
	if (!ret) { // Failed to initialize card
		_inited = -1;
		return -1;
	}

	if (!Fat16::init(&_card)) { // Failed to initialize volume
		_inited = -2;
		return -2;
	}
	_inited = 1;
	return 1;
}

uint8_t DLSD::get_num_files() {
	return NUM_FILES;
}

uint16_t DLSD::get_files_count(uint8_t fid) {
	return _files_count[fid];
}

uint8_t DLSD::set_files_count(uint8_t fid, uint16_t count) {
	_files_count[fid] = count;
	return 1;
}

void DLSD::reset_files_count() {
	for(uint8_t i = 0; i < NUM_FILES; i++)
		_files_count[i] = 0;
}

uint16_t DLSD::get_saved_count(uint8_t fid) {
	return _saved_count[fid];
}

uint8_t DLSD::set_saved_count(uint8_t fid, uint16_t count) {
	_saved_count[fid] = count;
	return 1;
}

void DLSD::reset_saved_count() {
	for(uint8_t i=0;i < NUM_FILES; i++)
		_saved_count[i] = 0;
}

int8_t DLSD::is_available() {
	return _inited;
}

void DLSD::pad_filename(char *filename, uint16_t c) {
	char nums[6];
	//itoa(c, nums, 10);
	fmtUnsigned(c, nums, 10);
	uint8_t a = strlen(nums);
	for(char i=0;i<a;i++) {
		filename[8-(a-i)] = nums[i];
	}	
}

bool DLSD::increment_file(uint8_t n) {
	close(n);
	_files_count[n] = _files_count[n] + 1;
	return true;
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
		if (n != 0)
			pad_filename(_filename, _files_count[n]);
		strcat_P(_filename, sd_filename_ext);		
		ret = _files[n].open(_filename, flags);
		if (!_files[n].isOpen())
			return -1;
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

int DLSD::read(uint8_t n, char *ptr, int len) {
	return _files[n].read(ptr, len);
}

int DLSD::read(uint8_t n, char *ptr, int len, char t) {
	int c = 0;
	while (c < len) {
		ptr[c] = _files[n].read();
		if (ptr[c] < 0 || ptr[c] == t) {
			if (ptr[c] < 0) {
				ptr[c] = 0;
				return -1;
			}
			else {
				ptr[c] = 0;
				return 0;
			}
		}
		c++;
	}
	ptr[c] = 0;
	return 1;
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


