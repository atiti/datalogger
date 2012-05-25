#include <NewSoftSerial.h>
#include <Time.h>
#include <DLCommon.h>
#include <DLGSM.h>

#define SYSLOG 1

//DLSD sd(true, 10);
DLGSM gsm;
//char line[100];
int c = 0;
int ret = 0;
char buffer[800];

int myCallback(char *line, int len) {
  Serial.print(line);
  return 0;
}

void setup() {
  int gsmret = 0;
  // Initialize serial console
  Serial.begin(57600);
  // Initialize GSM  module serial
  gsm.init(buffer, 800, 5);
  gsm.debug(1);
  //gsm.GSM_set_callback(myCallback); // Set the callback function to execute on received line
  
  // Initialize the GSM module with AT commands 
  Serial.println("All Done");
}
void loop() {
  // Request the net status
  gsm.GSM_request_net_status();
  
  int s = gsm.GPRS_check_conn_state();
  if (s >= GPRSS_IP_STATUS && s < GPRSS_PDP_DEACT) {
    Serial.print("Status: ");
    Serial.println(s);
    if (s > -1) {
      gsm.pwr_off();
      Serial.println("Powered off");
      delay(5000); 
    }
  } else {
    Serial.println("Bad GPRS status");
  }
  delay(1000);
}
