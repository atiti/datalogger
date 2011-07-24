#include <NewSoftSerial.h>
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
  while (gsmret == 0) {
     Serial.println("Initializing GSM module...");
     gsmret = gsm.GSM_init(); 
  }
  Serial.println("GSM done. Now GPRS...");
  gsmret = 0;
  // Initialize the GPRS context
  while (gsmret == 0) {
    Serial.println("Initializing GPRS...");
    gsmret = gsm.GPRS_init();
    Serial.println(gsmret);
  }
  Serial.println("All Done");
}
void loop() {
  // Request the net status
  gsm.GSM_request_net_status();
  
  int s = gsm.GPRS_check_conn_state();
  if (s >= GPRSS_IP_STATUS && s < GPRSS_PDP_DEACT) {
    ret = gsm.GPRS_connect("46.4.106.217", 80, true);
    if (ret == GPRSS_CONNECT_OK) {
        Serial.println("Connect OK");
        gsm.GPRS_send_start();
        gsm.GPRS_send("GET /test.php HTTP/1.0\r\n\r\n");
        gsm.GPRS_send_end();
        gsm.GSM_fast_read("CLOSED");
        ret = gsm.GPRS_close();
        if (ret)
          Serial.println("Close OK");
        else
          Serial.println("Close failed");
    } else
        Serial.println("Connect failed 2");
  } else {
    Serial.println("Bad GPRS status");
  }
  delay(1000);
}
