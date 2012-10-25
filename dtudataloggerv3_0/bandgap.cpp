#include <Arduino.h>
#include <DLCommon.h>

int toggle = 0;

void setup() {
	delay(100);
	Serial.begin(19200);
	Serial.println("Welcome!");
	pinMode(0, OUTPUT);
	digitalWrite(0, HIGH);
}

void loop() {
	Serial.print("Bandgap: ");
	Serial.print(get_bandgap(), DEC);
	Serial.println("V");
	delay(1000);
	for(int i=0;i<8;i++) {
		analogRead(i);
	}
	digitalWrite(0, toggle);
	if (toggle)
		toggle = 0;
	else
		toggle = 1;
}
