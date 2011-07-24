#include <Time.h>
#include <DLCommon.h>
#include <DLSD.h>

#define SYSLOG 1

DLSD sd(true, 10);
char line[100];
int c = 0;

void setup() {
  int8_t ret;
  Serial.begin(9600);
  ret = sd.init();
  if (ret == 1)
    Serial.println("SD init successful!");
  else
    Serial.println("SD init failed!");
}

void loop() {
  if (sd.is_available() > 0) {
    if (c < 20) {
      float stime = millis();
      unsigned long filesize = sd.open(DATALOG,  O_RDWR| O_CREAT | O_APPEND);
      sd.write(DATALOG, "Hello World ");
      sd.write(DATALOG, c);
      sd.write(DATALOG, "\n");
      Serial.print("Latency: ");
      Serial.println(millis()-stime);
      Serial.print("File size: ");
      Serial.println(filesize);
      Serial.print("Free mem: ");
      Serial.println(get_free_memory());
      delay(1000);
      sd.rewind(DATALOG);
      sd.read(DATALOG, line, 90, '\n');
      Serial.println(line); 
      sd.seekend(DATALOG);
      sd.close(DATALOG);
      c++;
    } else {
      c = 0;
      sd.increment_file(DATALOG);
    }
  } else {
   Serial.println("SD not initialized!");
   sd.init();
   delay(1000); 
  }
}
