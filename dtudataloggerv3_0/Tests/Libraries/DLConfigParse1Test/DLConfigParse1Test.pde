#include <Time.h>
#include <DLCommon.h>
#include <DLSD.h>

#define SYSLOG 1

DLSD sd(true, 10);
char line[100];
int c = 0;

int log_process_callback(char *line, int len) {
  char *param, *ptr;
  // Here we extract the name and value combination by finding the equal sign
  // Where the param pointer refers to the value, while the line refers to the name of the var
  ptr = strstr(line, "=");
  param = ptr + 1;
  *ptr = 0;

  Serial.println(line);

  if (strncmp_P(line, PSTR("PORT"), 4) == 0) {
    // The port number is extracted from the end of the name field
    ptr = line+10;
    //Serial.print("Port number: ");
    Serial.println(atoi(ptr));
    
    // Then we find out which port type we are talking about
    if (strncmp_P(line, PSTR("PORT_MOD"), 8) == 0) {
      //Serial.print("Port mode: ");
      Serial.println(param);
    } else if (strncmp_P(line, PSTR("PORT_TRIG"), 9) == 0) {
      //Serial.print("Port trigger: ");
      Serial.println(param); 
    }
  }

  return 0;
}

void setup() {
  int8_t ret;
  Serial.begin(57600);
  ret = sd.init();
  //if (ret == 1)
  //  Serial.println("SD init successful!");
  //else
  //  Serial.println("SD init failed!");
  sd.debug(1);
}

void loop() {
  if (sd.is_available() > 0) {
      float stime = millis();
      unsigned long filesize = sd.open(CONFIG,  O_READ);
    //  Serial.print("File size: ");
      Serial.println(filesize);
    //  Serial.print("Free mem: ");
      Serial.println(get_free_memory());
      sd.rewind(CONFIG);
      int rv = 0;
      do {
        rv = sd.read(CONFIG, line, 90, '\n');
        if (line[0] != '#' && line[0] != 0 && line[0] != '\n') {
          log_process_callback(line, 90);
        }
      }  while (rv >= 0);
      sd.close(CONFIG);
  } else {
   //Serial.println("SD not initialized!");
   sd.init();
  }
  int memory = memory_test();
  Serial.println(memory);
  delay(5000); 
}
