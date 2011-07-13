#include "WProgram.h"
#include "DLGSM.h"

prog_char gsm_string_0[] PROGMEM = "\r\nATE0\r\n";
prog_char gsm_string_1[] PROGMEM = "AT+GOI\r\n";
prog_char gsm_string_2[] PROGMEM = "AT+CREG=2\r\n";
prog_char gsm_string_3[] PROGMEM = "AT+CGREG=2\r\n";
prog_char gsm_string_4[] PROGMEM = "AT+CGREG?\r\n";
prog_char gsm_string_5[] PROGMEM = "AT+CGATT?\r\n";
prog_char gsm_string_6[] PROGMEM = "AT+CGATT=1\r\n";
prog_char gsm_string_7[] PROGMEM = "AT+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
prog_char gsm_string_8[] PROGMEM = "AT+CLPORT=\"TCP\",\"2020\"\r\n";
prog_char gsm_string_9[] PROGMEM = "AT+CSTT=\"internet.bibob.dk\",\"\",\"\"\r\n";
prog_char gsm_string_10[] PROGMEM = "AT+CIPSRIP=1\r\n";
prog_char gsm_string_11[] PROGMEM = "AT+CIICR\r\n";
prog_char gsm_string_12[] PROGMEM = "AT+CIFSR\r\n";
prog_char gsm_string_13[] PROGMEM = "AT+CIPSTART=\"TCP\",\"";
prog_char gsm_string_14[] PROGMEM = "AT+CIPSTART=\"UDP\",\"";
prog_char gsm_string_15[] PROGMEM = "AT+CIPSEND\r\n";
prog_char gsm_string_16[] PROGMEM = "AT+CIPCLOSE\r\n";
prog_char gsm_string_17[] PROGMEM = "AT+CIPSTATUS\r\n";
prog_char gsm_string_18[] PROGMEM = "AT+CLTS\r\n";
PROGMEM const char *gsm_string_table[] = { gsm_string_0, gsm_string_1, gsm_string_2,
                                           gsm_string_3, gsm_string_4, gsm_string_5,
                                           gsm_string_6, gsm_string_7, gsm_string_8,
                                           gsm_string_9, gsm_string_10, gsm_string_11,
                                           gsm_string_12, gsm_string_13, gsm_string_14,
                                           gsm_string_15, gsm_string_16, gsm_string_17,
                                           gsm_string_18 };

prog_char gprs_state_0[] PROGMEM = "STATE:";
prog_char gprs_state_1[] PROGMEM = "PDP";
prog_char gprs_state_2[] PROGMEM = "CONNECT";
prog_char gprs_state_3[] PROGMEM = "TCP CONNECT";
prog_char gprs_state_4[] PROGMEM = "UDP CONNECT";
prog_char gprs_state_5[] PROGMEM = "TCP CLOSED";
prog_char gprs_state_6[] PROGMEM = "IP STATUS";
prog_char gprs_state_7[] PROGMEM = "UDP CLOSED";
prog_char gprs_state_8[] PROGMEM = "";
prog_char gprs_state_9[] PROGMEM = "";

PROGMEM const char *gprs_state_table[] = { gprs_state_0, gprs_state_1, gprs_state_2,
                                           gprs_state_3, gprs_state_4, gprs_state_5,
                                           gprs_state_6, gprs_state_7, gprs_state_8};

NewSoftSerial _gsmserial(RX, TX);

DLGSM::DLGSM()
{
}
                
void DLGSM::debug(int v) {
}

void DLGSM::GSM_init() {
	
}

void DLGSM::GSM_process() {
}

void DLGSM::GSM_process_callback() {
}

void DLGSM::GSM_send(char b) {
}

void DLGSM::GSM_send(int v) {
}

void DLGSM::GSM_send(char *msg) {
}

void DLGSM::GSM_recvline(char *ptr, int len) {
	char curchar[2];
	int i = 0;
	// Read the first 2 bytes (should be the beginning of a reply)
	for(i=0;i<2;i++) {
    		curchar[i] = _gsmserial.read();
  	}
  	// Check that we got the top of the reply
  	if (curchar[0] != '\r' && curchar[1] != '\n') {
    		ptr[0] = curchar[0];
    		ptr[1] = curchar[1];
    		i = 2;
  	} else {
    		i = 0;
	}
	// Read the rest of the line
	while (curchar[0] != '\n' && i < len) {
		if (_gsmserial.available()) {
			curchar[0] = (char)_gsmserial.read();
			ptr[i] = curchar[0];
			i++;
		} else {
			ptr[i] = 0;
      			break;
    		}
 	}
 	ptr[i] = 0;
}

void DLGSM::GSM_request_net_status() {
}

void DLGSM::GSM_get_local_time() {
}
                
void DLGSM::GPRS_init() {
}

bool DLGSM::GPRS_connect(char *server, int port, bool proto) {
}
            
bool DLGSM::GPRS_send_start() {
}

void DLGSM::GPRS_send(char *data) {
}

void DLGSM::GPRS_send_raw(char *data, int len) {
}

void DLGSM::GPRS_send(float n) {
}

void DLGSM::GPRS_send(unsigned long n) {
}

void DLGSM::GPRS_send_end() {
}

void DLGSM::GPRS_close() {
}

void DLGSM::GPRS_check_conn_state() {

}


