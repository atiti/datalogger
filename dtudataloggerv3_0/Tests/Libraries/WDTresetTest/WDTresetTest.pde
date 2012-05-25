#include <avr/wdt.h>


char buffer[1200];
int cnt = 0;
int r = 0;

int demo(int a1) {
  int a, b,c, d, e;
  r++;
  a = a1;
  b = a*a + 1;
  c = a+b + buffer[cnt-10];
  d = c + b + a;
  e = d + a;
  buffer[cnt] = e;
  if (r < 10) {
    for(int k=0;k<cnt;k++) {
      demo(e);
    }
  }
}

void setup() {
 wdt_disable();
 Serial.begin(9600); 
 Serial.println("Booted");
 //wdt_enable(WDTO_8S);
}

void loop() {
  Serial.print("Test #");
  Serial.println(cnt);
  r = 0;
  demo(cnt);
  //wdt_reset();
  //delay(1000);
  cnt += 10;
}
