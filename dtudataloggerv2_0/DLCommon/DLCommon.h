#ifndef DLCommon_h
#define DLCommon_h

#include "WProgram.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WConstants.h"
#include <Time.h>

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

#endif

