#include <Time.h>
#include <DLCommon.h>
#include <DLAnalog.h>


DLAnalog analog(2, 3, 5, 4, 6, 14, LOW);
uint16_t analog_values[16] = { 0 };

char buff[100];


void setup() {
  Serial.begin(57600); 
  
  analog.init(analog_values);
  analog.debug(1); // Turn on analog debug  ging


  analog.set_pin(0, EVENT);
  analog.set_pin(1, DIGITAL);
  analog.set_pin(7, ANALOG);
  analog.set_pin(8, DIGITAL);
  analog.set_pin(9, ANALOG);
  analog.set_pin(13, ANALOG);
}

void loop() {
  
  uint8_t v = analog.read_all(); // Read all analog ports
  if (v >= 10) {
    analog.get_all();
    analog.time_log_line(buff);   
    analog.reset();
    Serial.print(buff);
    delay(200);
  }

  //int memory = memory_test();
  //Serial.println(memory);
  //Serial.println(analog_values[0]);
  //Serial.println(analog.get_voltage(analog_values, 0));
  
  //delay(1000);
}
