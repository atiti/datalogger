#ifndef DLCommon_h
#define DLCommon_h

#include "WProgram.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WConstants.h"

void get_from_flash(void *ptr, char *dst);
void get_from_flash_P(const prog_char *ptr, char *dst);
int strcmp_flash(char *str, void *ptr, char *dst);
int get_free_memory();
int get_checksum(char *string);

#endif

