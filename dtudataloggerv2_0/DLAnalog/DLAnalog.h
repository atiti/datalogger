#ifndef DLAnalog_h
#define DLAnalog_h

#include "WProgram.h"
#include <DLCommon.h>
#include <Time.h>

#define VREF 5.0
#define EXT_PWR_PIN 15 

#define IO_OFF 0
#define IO_ANALOG 1
#define IO_DIGITAL 2
#define IO_EVENT 3
#define IO_COUNTER 4

#define MEASURE_RATE 500
#define MEASURE_MULTIPLIER 2

typedef void (*INT_callback)();

class DLAnalog
{
	public:
		DLAnalog(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t en, uint8_t inp, bool pullup);
		void init(double *vals, double *std_dev, uint8_t *count_values, uint16_t measure_time);
		void debug(uint8_t v);
		void enable();
		void disable();
		void pwr_on();
		void pwr_off();
		void set_measure_time(uint16_t measure_time);
		uint16_t read(uint8_t pin);
		uint8_t read_all(uint8_t intr);
		uint8_t read_all();
		void reset();
		uint8_t get_all();
		void set_int_fun(INT_callback fun);
		void set_pin(uint8_t pin, uint8_t doa);
		uint8_t get_pin(uint8_t pin);
		float get_voltage(uint8_t pin);
		void time_log_line(char *line);
		void event_log_line(char *line);
		int get_bandgap();
	private:
		uint8_t _en, _inp;
		bool _pullup;
		uint16_t _sum_cnt; // Number of measurements
		uint16_t _measure_time; // Measurement length
		time_t _smeasure; // Measurement start time
		volatile uint16_t _dvals;
		uint8_t *_count_vals;
		double *_vals;
		double *_std_dev;
		uint32_t _count_start;
		uint8_t _DEBUG;
		uint8_t _s[4];
		uint8_t _AOD[16];
		int _bandgap;
		INT_callback _int_ptr;
};

#endif

