#ifndef DLCommon_h
#define DLCommon_h

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Time.h>
#include <pt.h>

#define USE_PT


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

#endif

