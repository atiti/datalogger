#include <DLAnalog.h>


DLAnalog analog(2, 3, 5, 4, 6, 14, LOW);
unsigned short analog_values[16] = { 0 };


void setup() {
  Serial.begin(9600); 
  analog.debug(1); // Turn on analog debug  ging
}

void loop() {
  float v = analog.readAll(analog_values); // Read all analog ports
  Serial.println(v);
  for(int i=0;i<16;i++) {
    Serial.print(analog_values[i]);
    Serial.print(" ");
  }
  Serial.println();
  //delay(1000);
}
