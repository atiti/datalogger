#include <Arduino.h>
#include <SdFat.h>
// include the library code:
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
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
SoftwareSerial _serial(18, 19);

char state = 0;

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;
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

void lcd_print_line(char *line) {
	lcd.print(line);
	cursor_line++;
	if (cursor_line > 1)
		cursor_line = 0;
	lcd.setCursor(0, cursor_line);
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

long read_hex_line(ifstream *file) {
	unsigned char c = 0;
	long read_bytes = 0;
	// Read the line
	do {
		c = file->get();
		if (c == 0xff) return 0;
		read_bytes++;
	} while (c != '\n' && c != '\r' && c != 0xff);

	// Finish up the line	
	do {
		c = file->get();
		if (c != 0xff) read_bytes++;
		else return 0;
	} while (c == '\n' || c == '\r');

	return read_bytes;
}

void program() {
  char c;
  char buff[24];
  long total_bytes = 0;
  long read_bytes = 0;
  // set current working directory
  if (!sd.chdir("/FIRMWARE/")) {
  	lcd_print_line("FIRMWARE dir missing");
	sd.errorHalt("chdir failed. Does the directory exist?");
  }
  // open file in current working directory
  ifstream file("CURRENT.HEX");

  if (!file.is_open()) { 
	lcd_print_line("open failed");
	sd.errorHalt("open failed");
  }

  // copy the file to Serial
  //while ((c = file.get()) >= 0) Serial.print(c);
  
  while ((read_bytes = read_hex_line(&file)) > 0) { 
	total_bytes += read_bytes;
	lcd_spinning(); 
	lcd.setCursor(0,0);
	sprintf(buff, "Loading: %d B", total_bytes);
	lcd.print(buff);
  };

  Serial.println("Done");
  lcd.setCursor(0,1);
  lcd.print("Done.");
}

void setup() {
	Serial.begin(19200);
	_serial.begin(115200);
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

void loop() {
	unsigned char c;
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd.setCursor(0, 1);
	// print the number of seconds since reset:
	lcd.print(millis()/1000);
	lcd_spinning();
	switch (state) {
		case 0:
			if (Serial.available()) {
				c = Serial.read();
				if (c == 'p')
					state = 1;
			}
			while (_serial.available()) {
				c = _serial.read();
				Serial.println(c, DEC);
			}
			break;
		case 1:
			program();
			lcd.clear();
			state = 0;
			break;
		default:
			break;
	}
	delay(100);
}
