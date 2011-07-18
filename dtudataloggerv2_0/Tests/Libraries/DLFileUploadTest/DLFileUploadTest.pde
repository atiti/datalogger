#include <NewSoftSerial.h>
#include <DLCommon.h>
#include <DLSD.h>
#include <DLGSM.h>
#include <DLHTTP.h>

#define BUFFSIZE 200
DLSD sd(true, 10);
DLGSM gsm;
DLHTTP http;

char buffer[BUFFSIZE];
char buffer2[201];
char smallbuff[10];

void setup() {
  int gsmret = 0;
  // Initialize serial console
  Serial.begin(57600);
  // Initialize GSM  module serial
  gsm.init(buffer, BUFFSIZE, 5);
  http.init(buffer, &gsm);
  gsm.debug(1);

  sd.debug(0);
  gsmret = sd.init();
  if (gsmret)
    Serial.println("SD init successful!");
  else
    Serial.println("SD init failed!");
    
}
void loop() {
  int ret = 1;
  unsigned long filesize = sd.open(DATALOG, O_RDWR);
  unsigned long sent_counter = 0;
  sd.rewind(DATALOG);
  int parts = 1, checksum = 0;
  if (filesize > 4000) {
    parts = filesize / 4000;   
  }
  for(int i = 0; i < parts; i++) {
    strcpy(buffer2, "http://attila.patup.com/dl/test.php?id=1");
    strcat(buffer2, "&part=");
    strcat(buffer2, itoa(i, smallbuff, 10));
    int cps = 4000;
    unsigned long stime = millis();
    sent_counter = 0;
    checksum = 0;
    if (http.POST_start(buffer2, cps)) {
      while (ret > 0 && sent_counter < cps) {
        memset(buffer2, 0, 200);
        ret = sd.read(DATALOG, buffer2, 200);
        checksum ^= get_checksum(buffer2);
        http.POST(buffer2); 
        sent_counter += ret;
      }  
      Serial.print("Checksum ");
      Serial.println(checksum);
      //http.POST("abcdefghijkl");
      http.POST_end();
      stime = millis() - stime;
      Serial.print("POST latency: ");
      Serial.println(stime);
      
    } else {
      http.POST_end(); 
      Serial.println("POST failed"); 
    }
  }
  sd.close(DATALOG);
  Serial.print("Free memory: ");
  Serial.println(get_free_memory());
  delay(1000);
}
