#include "WProgram.h"
#include "DLCommon.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WConstants.h"

extern unsigned int __bss_end;
extern void *__brkval;

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
	int free_memory;
	if ((int)__brkval == 0)
		free_memory = ((int)&free_memory) - ((int)&__bss_end);
	else
		free_memory = ((int)&free_memory) - ((int)__brkval);

	return free_memory;
}
int memory_test() {
	// This function will return the number of bytes currently free in SRAM
	int byteCounter = 0; // initialize a counter
	byte *byteArray; // create a pointer to a byte array
	// Use the malloc function to repeatedly attempt allocating a certain number of bytes to memory
	while ( (byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL ) {
		byteCounter++; // If allocation was successful, then up the count for the next try
		free(byteArray); // Free memory after allocating it
	}
	free(byteArray); // Also free memory after the function finishes
	return byteCounter; // Send back the number number of bytes which have been successfully allocated
}

// Very simple XOR checksum
uint8_t get_checksum(char *string) {
	int i;
	uint8_t XOR;
	uint8_t c;
	for (XOR = 0, i = 0; i < strlen(string); i++) {
		c = (uint8_t)string[i];
		XOR ^= c;
	}
	return XOR;
}

