#ifndef DLMeasure_h
#define DLMeasure_h

#include <Arduino.h>
#include <DLCommon.h>
#include <Time.h>

/* IO defines */
#define NUM_ANALOG 8  // Number of analog ports
#define NUM_DIGITAL 6  // Number of digital ports
#define ANALOG_OFFSET 0 // Offset to the analog values in the array
#define DIGITAL_OFFSET 8 // Offset to the digital values in the bufers

#define NUM_IO (NUM_ANALOG+NUM_DIGITAL) // Total number of IO ports

/* PC Interrupt definition */
#define DIGITAL_PORT PORTC 
#define DIGITAL_PCIE PCIE2
#define DIGITAL_PCMSK PCMSK2 // PCINT23-16
#define DIGITAL_PCMSK_VAL 0xF8  // pins 23-18
#define DIGITAL_ISR_VECT PCINT2_vect

/* Voltage reference */
#define VREF 5.0
#define EXT_PWR_PIN 15 

/* IO port types */
#define IO_OFF 0
#define IO_ANALOG 1
#define IO_DIGITAL 2
#define IO_EVENT 3
#define IO_COUNTER 4

#define MEASURE_RATE 500
#define MEASURE_MULTIPLIER 2

typedef void (*INT_callback)();

const char num2pin_mapping[] = { 24, // AI0
				25, // AI1
				26, // AI2
				27, // AI3
				28, // AI4
				29, // AI5
				30, // AI6
				31, // AI7
				23, // D23
				22, // D22
				21, // D21
				20, // D20
				19, // D19
				18 // D18
				};

class DLMeasure
{
	public:
		DLMeasure();
		void init();
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
		uint32_t _count_start;
		uint8_t _DEBUG;
		INT_callback _int_ptr;
};

#endif

