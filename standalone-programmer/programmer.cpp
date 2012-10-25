#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <SdFat.h>
// include the library code:
#include <LiquidCrystal.h>
//#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include "stk500.h"
/* 

4 - RS (7)
6 - En (6)
12 - DB5 (5)
11 - DB4 (4)
14 - DB7 (3)
13 - DB6 (2)

*/

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 4, 5, 2, 3);
// Programming target serial
//SoftwareSerial _serial(18, 19);
AltSoftSerial _serial;

//#define prog_serial _serial
#define prog_serial Serial


char state = 0;
long sketchLength = 19142;
// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;
SdFile firmware;

int cnt = 0;

/* LCD functions */
char cursor_line = 0;
char spin_pos_x = 22;
char spin_pos_y = 0;
char spin_val = 0;

// make some custom characters:
byte backslash[8] = {
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000,
  0b00000
};
void lcd_reset_line() {
	cursor_line = 0;
}

void lcd_print_line(char *line) {
	lcd.setCursor(0, cursor_line);
	lcd.print(line);
	cursor_line++;
	if (cursor_line > 1)
		cursor_line = 0;
}

void lcd_spinning() {
	char c = ' ';
	lcd.setCursor(spin_pos_x-1, spin_pos_y);
	lcd.print("[ ]");
	lcd.setCursor(spin_pos_x, spin_pos_y);
	if (spin_val == 0) c = '|';
	else if (spin_val == 1) c = '/';
	else if (spin_val == 2) c = '-';
	else if (spin_val == 3) c = 0; 
	spin_val++;
	if (spin_val == 4) spin_val = 0;
	lcd.write(c);
}
/* End of LCD functions */

long i,j,start,end,address,laddress,haddress;
int began = 0, groove = 50, error = 0, a, b, c, d, e, f, buffLength, k, preBuff, readBuffLength;
char writeBuff[16],readBuff[16], buff[128], line[40];
int fillbuff;

// Reads bytes until there's nothing left to read
// Then sticks 'em in readBuff, and sets readBuffLength
void readBytes() {
  readBuffLength = 0;
  while (prog_serial.available() > 0) {
    preBuff = prog_serial.read();
  // this if statement is necessary because for some reason, the target
    // board replies with 0xFC instead of 0x10
    // i've got no idea whether it's an error code, but simply substituting
    // 0xFC for 0x10 in the readBuff seems to do the trick
    if (preBuff == 0xFC) {
      preBuff = 0x10;
    }
    readBuff[readBuffLength] = preBuff;
    readBuffLength++;
  }
}

// Read bytes until there's nothing left to read
// The only difference to the above function is that
// this one doesn't substitute 0xFC for 0x10
void readBytess() {
  readBuffLength = 0;
  while (prog_serial.available() > 0) {
    readBuff[readBuffLength] = prog_serial.read();
    readBuffLength++;
  }

}

long read_hex_line(char *buff, int len) {
	unsigned char c = 0;
	long read_bytes = 0;
	

	// Read the line
	do {
		c = firmware.read();
		if (c == 0xff) return 0;
                if (c != '\n' && c != '\r' && c != ':' && read_bytes < len) {
                        *(buff+read_bytes) = c;
			read_bytes++;
		}
	} while (c != '\n' && c != '\r' && c != 0xff);

	*(buff+read_bytes) = 0;

	// Finish up the line	
	do {
		c = firmware.read();
		if (c != 0xff) read_bytes++;
		else return 0;
	} while (c == '\n' || c == '\r');

	return read_bytes;
}

int process_read_byte(char *buff, int pos, char *ret) {
	char hex[3];
	int p = pos;

/*	if (*ptr == '\n' || *ptr == '\r' || *ptr == ':') {
		ptr++;
		*ret = -1;
		return ptr;
	}
*/
	hex[0] = *(buff+p);
	p++;
	hex[1] = *(buff+p);
	p++;
	hex[2] = 0;
	sscanf(hex, "%2x", ret);
	return p;
}

int process_hex_line(char *buff, int len) {
	char *ptr = buff;
	int p = 0;
	char byte_count, c,i;
	short chksum = 0;
	uint8_t addrl,addrh;
	// Got data length
	p = process_read_byte(buff, p, &c);
	byte_count = c;
	chksum += c;
	// Got address	
	p = process_read_byte(buff, p, &c);
	addrh = c;
	chksum += c;
	p = process_read_byte(buff, p, &c);
	addrl = c;
	chksum += c;
	address = (addrh << 8) | addrl;
	// Record type
	p = process_read_byte(buff, p, &c);
	chksum += c;
	// Read data chunk
	for(i=0;i<byte_count;i++) {
		p = process_read_byte(buff, p, &c);
		chksum += c;
		writeBuff[i] = c;
	}
	// chksum
	p = process_read_byte(buff, p, &c);
	chksum += c;
	lcd_spinning();
	if ((chksum & 0xFF) == 0) {
		memcpy(buff+fillbuff, writeBuff, byte_count);
		fillbuff += byte_count;
/*                haddress = address / 256;
                laddress = address % 256;	
		lcd.setCursor(0,0);
		lcd.print(haddress, HEX);
		lcd.print(" ");
		lcd.print(laddress, HEX);
		lcd.print("          ");

		prog_serial.write((uint8_t)STK_LOAD_ADDRESS);
                prog_serial.write((uint8_t)laddress);
                prog_serial.write((uint8_t)haddress);
                prog_serial.write((uint8_t)CRC_EOP);
                delay(50);
    
                readBytes();
                if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
                	lcd_print_line("N5.1");
                        delay(2500);
			return;
		}

                // Write the block
                prog_serial.write((uint8_t)STK_PROG_PAGE);
              	prog_serial.write((uint8_t)0x00);
                prog_serial.write((uint8_t)byte_count);
                prog_serial.write((uint8_t)0x46);
                for (i = 0; i < byte_count; i++) {
                	prog_serial.write((uint8_t)writeBuff[j]);
                }
                prog_serial.write((uint8_t)CRC_EOP);
                delay(50);
    
                readBytes();
                if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
                	lcd_print_line("N5.2");
                        delay(2500);
                	return;
		}
*/
	} else {
		lcd.print("Checksum fail ");
		lcd.print((uint8_t)(chksum & 0xff), HEX);
		lcd.print(" ");
		lcd.print(c, HEX);
		delay(2500);
	}
	return 0;
}

char firmware_read_byte(uint8_t *c) {
        char hex[3];
        char chr;
        do {
                chr = firmware.read();
        } while (chr == '\n' || chr == '\r' || chr == ':');
        if (chr == -1) return 0;

        hex[0] = chr;
        hex[1] = firmware.read();
        hex[2] = 0;

        sscanf(hex, "%2x", c);
        return 1;
}

long load_firmware() {
  long firmware_size = 0;
  // set current working directory
  if (!sd.chdir("/FIRMWARE/")) {
        lcd_print_line("FIRMWARE dir missing");
        sd.errorHalt("chdir failed. Does the directory exist?");
  }
  // open file in current working directory
  if (!firmware.open("CURRENT.HEX", O_READ)) {
        lcd_print_line("open failed");
        sd.errorHalt("open failed");
  }

  uint8_t u, r;
  while ((r = firmware_read_byte(&u)) > 0) {
  	firmware_size++;
  }
  Serial.print("FS: ");
  Serial.println(firmware_size); 
  firmware.seekSet(0);

  return firmware_size;
}

void close_firmware() {
  firmware.close();
  lcd.setCursor(0,1);
  lcd.print("Done.");
}

long fetch_firmware_part(char *buff, int len) {
	long total_bytes = 0;
	uint8_t u, r;
	while ((r = firmware_read_byte(&u)) > 0 && total_bytes <  len) {
		buff[total_bytes] = u;
		total_bytes++;
	}
	return total_bytes;
}

void program() {
  char c, stop = 0;
  //char buff[64];
  long total_bytes = 0;
  long read_bytes = 0;
  Serial.println("opening"); 
  load_firmware();
  Serial.println("reading");

  while (!stop) {
	read_bytes = fetch_firmware_part(buff, 128);
	Serial.print("got: ");
	Serial.println(read_bytes);	
	total_bytes += read_bytes;
	if (read_bytes == 0)
		stop = 1;
	lcd_spinning();
	lcd.setCursor(0,0);  	
	lcd.print("bytes: ");
	lcd.print(total_bytes);
  }

/*  while ((read_bytes = read_hex_line(&buff[0], 128)) > 0) { 
	total_bytes += read_bytes;
	lcd_spinning(); 
	lcd.setCursor(0,0);
	Serial.println(buff);
	sprintf(buff, "Loading: %d B", total_bytes);
	lcd.print(buff);
  };
*/
  close_firmware();
  delay(2500);
}

void setup() {
	Serial.begin(57600);
	_serial.begin(57600);
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	lcd.createChar(0, backslash);
	delay(100);
	// Print a message to the LCD.
	lcd_print_line("DL programmer v0.1");
		
	while (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
		lcd.setCursor(0, 1);
		lcd.print("SD card missing!");
		lcd_spinning();
	}
	lcd.clear();

	// PWR to teh board
	pinMode(A0, OUTPUT);
	digitalWrite(A0, HIGH);

	// RESET
	pinMode(A3, OUTPUT);
	digitalWrite(A3, LOW);
	delay(1000);
	digitalWrite(A3, HIGH);

	// RX - A4
	// TX - A5

}

void reset_target() {
	prog_serial.flush();
	digitalWrite(A3, LOW);
	delay(100);
	digitalWrite(A3, HIGH);
	prog_serial.flush();
}

void get_in_sync() {
	int i;
	for(i=0;i<25;i++) {
		prog_serial.write(STK_GET_SYNC);
		prog_serial.write(CRC_EOP);
		delay(50);
	}
}

void write_flash(int size) {
	haddress = address / 256;
        laddress = address % 256;       
        lcd.setCursor(0,0);
        lcd.print(haddress, HEX);
        lcd.print(" ");
        lcd.print(laddress, HEX);
        lcd.print("          ");

        prog_serial.write((uint8_t)STK_LOAD_ADDRESS);
        prog_serial.write((uint8_t)laddress);
        prog_serial.write((uint8_t)haddress);
        prog_serial.write((uint8_t)CRC_EOP);
        delay(50);
    
        readBytes();
        if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
        	lcd_print_line("N5.1");
                delay(2500);
                return;
        }

        // Write the block
        prog_serial.write((uint8_t)STK_PROG_PAGE);
        prog_serial.write((uint8_t)0x00);
        prog_serial.write((uint8_t)size);
        prog_serial.write((uint8_t)0x46);
        for (i = 0; i < size; i++) {
        	prog_serial.write((uint8_t)buff[i]);
        }
        prog_serial.write((uint8_t)CRC_EOP);
        delay(50);
    
        readBytes();
        if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
        	lcd_print_line("N5.2");
                delay(2500);
                return;
        }
}

void loop() {
	unsigned char c, stop;
	long read_bytes, total_bytes;
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd.setCursor(0, 1);
	// print the number of seconds since reset:
	lcd.print(millis()/1000);
	lcd_spinning();
	switch (state) {
		case 0:
			lcd.setCursor(0,0);
			if (prog_serial.available()) {
				while (prog_serial.available()) {
					c = prog_serial.read();
					lcd.write(c);
				}
			/*	cnt++;
				if (cnt == 5)
					state = 2;
			*/
			}
			
			if (((millis() / 1000) % 20) == 0)
				state = 2;

			break;
		case 1:
			Serial.println("prog");
			program();
			Serial.println("done");
			lcd.clear();
			state = 0;
			break;
		case 2:
			cnt = 0;
			sketchLength = load_firmware();
			lcd.clear();
			lcd_reset_line();
			lcd_print_line("Reseting target");
			lcd_spinning();
			reset_target();	
			get_in_sync();
			readBytes();
			// We expect to receive 0x14 (STK_INSYNC) and 0x10 (STK_OK)
			// Every response must start with 0x14 and end with 0x10, otherwise
			// something's wrong
			if (readBuffLength < 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
				lcd_print_line("No sync");
				state = 3;
				break;
			}		

			lcd_spinning();
			prog_serial.flush();
			
			// Set the programming parameters
			prog_serial.write((uint8_t)STK_SET_DEVICE);
			prog_serial.write((uint8_t)0x86);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)0x01);
			prog_serial.write((uint8_t)0x01);
			prog_serial.write((uint8_t)0x01);
			prog_serial.write((uint8_t)0x01);
			prog_serial.write((uint8_t)0x03);
			prog_serial.write((uint8_t)0xff);
			prog_serial.write((uint8_t)0xff);
			prog_serial.write((uint8_t)0xff);
			prog_serial.write((uint8_t)0xff);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)0x80);
			prog_serial.write((uint8_t)0x04);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)0x80);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)CRC_EOP);
			lcd_spinning();
			delay(50);

			readBytes();
			if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
			        lcd_print_line("N2");
                                state = 3;
                                break;
			}

			lcd_spinning();

			// Set the extended programming parameters
			prog_serial.write((uint8_t)STK_SET_DEVICE_EXT);
			prog_serial.write((uint8_t)0x05);
			prog_serial.write((uint8_t)0x04);
			prog_serial.write((uint8_t)0xd7);
			prog_serial.write((uint8_t)0xc2);
			prog_serial.write((uint8_t)0x00);
			prog_serial.write((uint8_t)CRC_EOP);
			lcd_spinning();
			delay(50);
  
			readBytes();
			if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
			      lcd_print_line("N3");
                              state = 3;
                              break;
			}

			lcd_spinning();
			lcd_print_line("Now wer talkin.");			

			// Enter programming mode
			prog_serial.write((uint8_t)STK_ENTER_PROGMODE);
			prog_serial.write((uint8_t)CRC_EOP);
			delay(50);
			lcd_spinning();  

			readBytes();
			if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
			        lcd_print_line("N4");
                        	state = 3;
                                break;
			}
			lcd_spinning();

			// The actual flashing
			// Now comes the interesting part
			// We put blocks of data from our Arduino's flash and into buff, ready to write
			// to the target Arduino's flash
			// We only write in blocks of 128 or less bytes
			lcd.setCursor(0,1);
			lcd.print("Sketch: ");
			lcd.print(sketchLength);
			address = 0;
			stop = 0;
			fillbuff = 0;
			while ((read_bytes = read_hex_line(&line[0], 40)) > 0) { 
				if (fillbuff < 256) {
					stop = process_hex_line(line, read_bytes);
					total_bytes += read_bytes;
				} else {
					write_flash(fillbuff);	
					fillbuff = 0;
					stop = process_hex_line(line, read_bytes);
					total_bytes += read_bytes;
					lcd_spinning();
					address += 64;
				}			
				/*lcd.setCursor(0,0);
				sprintf(buff, "Loading: %d B", total_bytes);
				lcd.print(buff);*/
			}
			if (fillbuff > 0) {
				write_flash(fillbuff);
			}	

/*	

			for (i = 0; i < sketchLength; i += 128) {
*/
				/*start = i;
				end = i + 127;
				if (sketchLength <= end) {
					end = sketchLength - 1;
				}
				buffLength = end - start + 1;
				for (j = 0; j < buffLength; j++) {
					buff[j] = pgm_read_byte(i+j);
				}*/
/*				
				buffLength = fetch_firmware_part(buff, 128);
				lcd.setCursor(0,1);
				lcd.print(buffLength);
				// The buffer is now filled with the appropriate bytes
    
				// Set the address of the avr's flash memory to write to
				haddress = address / 256;
				laddress = address % 256;
				address += 64; // For the next iteration
				prog_serial.write((uint8_t)STK_LOAD_ADDRESS);
				prog_serial.write((uint8_t)laddress);
				prog_serial.write((uint8_t)haddress);
				prog_serial.write((uint8_t)CRC_EOP);
				lcd_spinning();
				delay(50);
    
				readBytes();
				if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
				        lcd_print_line("N5.1");
                                        delay(2500);
                                        state = 0;
                                        break;
				}

				// Write the block
				prog_serial.write((uint8_t)STK_PROG_PAGE);
				prog_serial.write((uint8_t)0x00);
				prog_serial.write((uint8_t)buffLength);
				prog_serial.write((uint8_t)0x46);
				for (j = 0; j < buffLength; j++) {
					prog_serial.write((uint8_t)buff[j]);
				}
				prog_serial.write((uint8_t)CRC_EOP);
				lcd_spinning();
				delay(50);
    
				readBytes();
				if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
				        lcd_print_line("N5.2");
                                	delay(2500);
                            	        state = 0;
	                                break;
				}
				lcd.setCursor(0,0);
				lcd.print("Prog: ");
				lcd.print(i);
			}
*/

			// Leave programming mode
			prog_serial.write((uint8_t)STK_LEAVE_PROGMODE);
			prog_serial.write((uint8_t)CRC_EOP);
			lcd_spinning();
			delay(50);
			readBytes();
			if (readBuffLength != 2 || readBuff[0] != 0x14 || readBuff[1] != 0x10) {
				lcd_print_line("N6");
				state = 3;
				break;			
			}
			lcd_spinning();
			close_firmware();
			reset_target();
			delay(2500);
			lcd.clear();
			state = 0;
			break;
		case 3:
			delay(2500);
			close_firmware();
			state = 0;
			break;
		default:
			break;
	}
	delay(100);
}
