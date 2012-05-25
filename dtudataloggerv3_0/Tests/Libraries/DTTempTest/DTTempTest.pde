#include <DLAnalog.h>
#include <DLTemp.h>

DLAnalog analog(2, 3, 5, 4, 6, 14, LOW); // s0, s1, s2, s3, en, inp, pullup
unsigned short analog_values[16] = { 0 };

DLTemp temp(2.7, 5.0, -0.03); // R1 kOhm, Vref, offset

void setup() {
  Serial.begin(9600); 
  analog.debug(1); // Turn on analog debug  ging
  temp.init(7, analog_values); // position of temp and array of values
}

void loop() {
  float v = analog.readAll(analog_values); // Read all analog ports
  Serial.print("Current temperature: ");
  Serial.print(temp.getTemp());
  Serial.println("C");
  //delay(1000);
}
