#include <Arduino.h>
#include <DLCommon.h>


void setup() {
	delay(1000);
	Serial.begin(57600);
	Serial.println("Welcome!");
}

void loop() {
	Serial.print("Bandgap: ");
	Serial.print(get_bandgap(), DEC);
	Serial.println("V");
	delay(1000);
	for(int i=0;i<8;i++) {
		analogRead(i);
	}
}
