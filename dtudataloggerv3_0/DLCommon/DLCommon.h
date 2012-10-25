#ifndef DLCommon_h
#define DLCommon_h

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <Time.h>
#include <pt.h>

#define USE_PT

#define _cons_serial Serial

// Logging macro, can be disabled on production
#define LOG(v) {  \
        _cons_serial.print(millis()); \
        _cons_serial.print(": "); \
        _cons_serial.println(v); \
        }

#define DEBUG_LOG(v) { \
        _cons_serial.print(__FILE__); \
        _cons_serial.print(":"); \
        _cons_serial.print(__LINE__); \
        _cons_serial.print(" "); \
        LOG(v); \
        }


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

typedef int (*FUN_callback)(char *, int);

void get_from_flash(void *ptr, char *dst);
void get_from_flash_P(const prog_char *ptr, char *dst);
int strcmp_flash(char *str, void *ptr, char *dst);
int get_free_memory();
uint8_t get_checksum(char *string);
int memory_test();
void fmtDouble(double val, byte precision, char *buf, unsigned bufLen = 0xffff);
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen = 0xffff, byte width = 0);
unsigned long crc_string(char *s);
unsigned long crc_struct(char *s, int len);
void set_supply_voltage(long v);
long get_supply_voltage();
void set_bandgap(long iref, int offset);
int get_bandgap();
void reboot_now();
void digital_clock_display();
void print_digits(int d);
#endif

