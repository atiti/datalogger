#include <avr/sleep.h>
#include <Arduino.h>
#include <DLCommon.h>

int ledPin = 0;
int interruptPin = 14; 
int sleepStatus = 0;
int count = 0;

void wakeUpNow() {

}

void setup() {
	delay(1000);
	pinMode(ledPin, OUTPUT);         // sets the digital pin as output
  	pinMode(interruptPin, OUTPUT);   //
	Serial.begin(57600);
	attachInterrupt(0, wakeUpNow, LOW);
	Serial.println("Welcome!");
}

void sleepNow() {
	pinMode(ledPin, INPUT);
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	sleep_enable();
	attachInterrupt(0, wakeUpNow, LOW);
	sleep_mode();
	sleep_disable();
	detachInterrupt(0);
	pinMode(ledPin, OUTPUT);
}

void loop() {
  // display information about the counter
  Serial.print("Awake for ");
  Serial.print(count);
  Serial.println("sec");
  count++;
  delay(1000);                           // waits for a second
 
  // compute the serial input
  if (Serial.available()) {
    int val = Serial.read();
    if (val == 'S') {
      Serial.println("Serial: Entering Sleep mode");
      delay(100);     // this delay is needed, the sleep
                      //function will provoke a Serial error otherwise!!
      count = 0;
      sleepNow();     // sleep function called here
    }
    if (val == 'A') {
      Serial.println("Hola Caracola"); // classic dummy message
    }
  }
 
  // check if it should go to sleep because of time
  if (count >= 10) {
      Serial.println("Timer: Entering Sleep mode");
      delay(100);     // this delay is needed, the sleep
                      //function will provoke a Serial error otherwise!!
      count = 0;
      sleepNow();     // sleep function called here
  }
}
