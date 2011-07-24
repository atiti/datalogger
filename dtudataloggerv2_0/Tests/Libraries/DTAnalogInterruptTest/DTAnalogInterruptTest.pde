#include <avr/interrupt.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <DLCommon.h>
#include <DLAnalog.h>


DLAnalog analog(17, 16, 5, 4, 6, 14, LOW);
volatile uint16_t analog_values[16] = { 0 };
volatile uint32_t std_dev[16] = { 0 };
volatile uint8_t got_event = 0;
long stime = millis();

char buff[200];

void intnow() {
  analog.read_all(1);
  got_event = 1;
}

void setup() {
  Serial.begin(57600);
 
  setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");      

  analog.init(analog_values, std_dev);
  analog.debug(1); // Turn on analog debug  ging

  analog.set_pin(0, EVENT);
  //analog.set_pin(1, ANALOG);
  //analog.set_pin(2, ANALOG);
  //analog.set_pin(3, ANALOG);
  //analog.set_pin(4, ANALOG);
  analog.set_pin(5, COUNTER);
  //analog.set_pin(6, ANALOG);
  analog.set_pin(7, ANALOG);
  analog.set_pin(8, COUNTER);
  //analog.set_pin(9, ANALOG);
  //analog.set_pin(10, ANALOG);
  analog.set_pin(11, DIGITAL);
  
  //analog.set_pin(2, DIGITAL);
  //analog.set_pin(7, ANALOG);
  //analog.set_pin(8, DIGITAL);
  //analog.set_pin(9, ANALOG);
  //analog.set_pin(13, ANALOG);

  attachInterrupt(0, intnow, CHANGE);
}

void loop() {
  uint8_t v = analog.read_all(); // Read all analog ports
  if (v >= 10) {
    if (analog.get_all() || (millis() - stime) > 1000) {
      stime = millis();
      analog.time_log_line(buff);   
      Serial.print(buff);   
    }
    analog.reset();
  }
  if (got_event) {
    got_event = 0;
    analog.event_log_line(buff);
    Serial.print(buff);
  }

  //Serial.println(analog_values[0]);
  //Serial.println(analog.get_voltage(analog_values, 0));
  
  //delay(1000);
}
