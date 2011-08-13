#include <Wire.h>
#include <NewSoftSerial.h>
#include <DS1307RTC.h>
#include <DLCommon.h>
#include <DLGSM.h>
#include <DLHTTP.h>
#include <Time.h>

#define SYSLOG 1

#define BUFFSIZE 200
//DLSD sd(true, 10);
DLGSM gsm;
DLHTTP http;

char buffer[BUFFSIZE];
char buffer2[100];

void setup() {
  int gsmret = 0;
  // Initialize serial console
  Serial.begin(57600);
  // Initialize GSM  module serial
  gsm.init(buffer, BUFFSIZE, 5);
  http.init(buffer, &gsm);
  gsm.debug(1);
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(day());
  Serial.print("-");
  Serial.print(month());
  Serial.print("-");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void loop() {
  strcpy(buffer2, "http://dl.zsuatt.com/data/2011/");
  uint8_t ret = http.GET(buffer2);
  Serial.print("HTTP Request: ");
  Serial.println(ret, DEC);
  //unsigned long t = HTTP_get_time();
  //setTime(t);
  digitalClockDisplay();
  Serial.print("Free memory: ");
  Serial.println(get_free_memory());
  int memory = memory_test();
  Serial.println(memory);
  //http.POST("abcdefghijkl");
  //http.POST_end();
  delay(1000);
}
