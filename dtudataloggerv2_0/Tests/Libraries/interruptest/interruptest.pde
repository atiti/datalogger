#include <avr/interrupt.h>

volatile char gotone = 0;

void intnow() {
   gotone = 1;
}

void setup() {
  pinMode(2, INPUT);
  Serial.begin(57600); 
  attachInterrupt(0, intnow, CHANGE);
}

void loop() {
  Serial.println("hi!");
  if (gotone) {
   Serial.println("Got one!");
   gotone = 0; 
  }
  delay(1000);
  
}
