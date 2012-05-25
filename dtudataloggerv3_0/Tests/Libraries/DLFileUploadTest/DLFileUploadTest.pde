#include <NewSoftSerial.h>
#include <DLCommon.h>
#include <DLSD.h>
#include <DLGSM.h>
#include <DLHTTP.h>
#include <Time.h>

#define DEBUG 1

#define BUFFSIZE 60
DLSD sd(true, 10);
DLGSM gsm;
DLHTTP http;

char buffer[BUFFSIZE];
char buffer2[201];
char smallbuff[10];

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

uint8_t upload_file(uint8_t fd) {
  unsigned long filesize = 0;
  unsigned long sent_counter = 0;
  short partlength = 4000;
  uint8_t err = 0, ret = 1;
  int parts = 1, i = 0;
  uint8_t checksum = 0;
  filesize = sd.open(fd, O_READ); // Open file read only
  sd.rewind(fd); // Rewind to the beginning
 
  // Split up files into multi parts
  if (filesize > partlength) { 
    parts = filesize / partlength;   
  }
 
  i = 0;
  while(i < parts) { // Send parts
    strcpy(buffer2, "http://attila.patup.com/dl/test.php?id=1");
    strcat(buffer2, "&part=");
    strcat(buffer2, itoa(i, smallbuff, 10));
    strcat(buffer2, "&totpart=");
    strcat(buffer2, itoa(parts, smallbuff, 10));
    int cps = 4000;
    unsigned long stime = millis();
    sent_counter = 0;
    checksum = 0;
    if (http.POST_start(buffer2, cps)) {
      err = 0; // Reset error counter, we should be good to go again
      while (ret > 0 && sent_counter < cps) {
        memset(buffer2, 0, 200);
        // Read the next chunk
        ret = sd.read(fd, buffer2, 200);
        // Calculate XOR checksum of the chunk
        checksum ^= get_checksum(buffer2);
        // POST the chunk
        http.POST(buffer2); 
        sent_counter += ret;
      }  
      http.POST_end();
      stime = millis() - stime;
#if DEBUG
      Serial.print("POST latency: ");
      Serial.println(stime);
      digitalClockDisplay();
#endif
      i++;
    } else {
#if DEBUG 
      Serial.println("POST failed!");
#endif 
      err++; // Increment error counter
      if (err > 5)
        return 0;
    }
  }
  sd.close(fd);
  return 1;
}

void loop() {
  int ret = 1;
  Serial.println(get_free_memory());
  ret = upload_file(CONFIG);
  int memory = memory_test();
  Serial.println(memory);

  ret = upload_file(DATALOG);
  
  delay(1000);
}
