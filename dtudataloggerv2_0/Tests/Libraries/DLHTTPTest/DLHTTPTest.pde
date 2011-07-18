#include <NewSoftSerial.h>
#include <DLCommon.h>
#include <DLGSM.h>
#include <DLHTTP.h>

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
void loop() {
  strcpy(buffer2, "http://attila.patup.com/test.php?id=1");
  uint8_t ret = http.GET(buffer2);
  Serial.print("HTTP Request: ");
  Serial.println(ret);
  Serial.print("Free memory: ");
  Serial.println(get_free_memory());
  //http.POST("abcdefghijkl");
  //http.POST_end();
  delay(1000);
}
