#ifndef DLAnalog_h
#define DLAnalog_h

#include "WProgram.h"
#include <Time.h>

#define VREF 5.0

#define OFF 0
#define ANALOG 1
#define DIGITAL 2
#define EVENT 3
#define COUNTER 4

class DLAnalog
{
	public:
		DLAnalog(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t en, uint8_t inp, bool pullup);
		void init(volatile uint16_t *vals, volatile uint32_t *std_dev);
		void debug(uint8_t v);
		void enable();
		void disable();
		uint16_t read(uint8_t pin);
		uint8_t read_all(uint8_t intr);
		uint8_t read_all();
		void reset();
		uint8_t get_all();
		void set_pin(uint8_t pin, uint8_t doa);
		uint8_t get_pin(uint8_t pin);
		float get_voltage(uint8_t pin);
		void time_log_line(char *line);
		void event_log_line(char *line);
	private:
		uint8_t _en, _inp;
		bool _pullup;
		uint8_t _sum_cnt;
		volatile uint16_t _dvals;
		volatile uint16_t *_vals;
		volatile uint32_t *_std_dev;
		uint8_t _DEBUG;
		uint8_t _s[4];
		uint8_t _AOD[16];
};

#endif

