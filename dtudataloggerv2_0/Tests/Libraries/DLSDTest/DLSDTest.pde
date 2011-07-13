#include <DLCommon.h>
#include <DLSD.h>

#define SYSLOG 1

DLSD sd(true, 10);
char line[100];
int c = 0;

void setup() {
  uint8_t ret;
  Serial.begin(9600);
  ret = sd.init();
  if (ret)
    Serial.println("SD init successful!");
  else
    Serial.println("SD init failed!");
}
void loop() {
  float stime = millis();
  unsigned long filesize = sd.open(SYSLOG,  O_RDWR| O_CREAT | O_APPEND);
  sd.write(SYSLOG, "Hello World ");
  sd.write(SYSLOG, c);
  sd.write(SYSLOG, "\n");
  Serial.print("Latency: ");
  Serial.println(millis()-stime);
  Serial.print("File size: ");
  Serial.println(filesize);
  Serial.print("Free mem: ");
  Serial.println(get_free_memory());
  delay(1000);
  sd.rewind(SYSLOG);
  sd.read(SYSLOG, line, 90, '\n');
  Serial.println(line); 
  sd.seekend(SYSLOG);
  //sd.close(SYSLOG);
  c++;
}
