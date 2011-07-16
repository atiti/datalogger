#include "WProgram.h"
#include "DLCommon.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WConstants.h"

void get_from_flash(void *ptr, char *dst) {
	strcpy_P(dst, (prog_char *)pgm_read_word(ptr));
}

void get_from_flash_P(const prog_char *ptr, char *dst) {
	strcpy_P(dst, ptr);
}

int strcmp_flash(char *str, void *ptr, char *dst) {
	get_from_flash(ptr, dst);	
	return strncmp(str, dst, strlen(dst));
}

int get_free_memory() {
	extern unsigned int __bss_end;
	extern void *__brkval;
	int free_memory;
	if ((int)__brkval == 0)
		free_memory = ((int)&free_memory) - ((int)&__bss_end);
	else
		free_memory = ((int)&free_memory) - ((int)__brkval);
	return free_memory;
}

