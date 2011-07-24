#include "WProgram.h"
#include "DLGSM.h"

#define Pchar prog_char PROGMEM

#define GSM_INIT_LEN 7
Pchar gsm_init_string_0[] = "AT\r\n";
Pchar gsm_init_string_1[] = "ATE0\r\n";
#ifdef GSM_SW_FLOW
Pchar gsm_init_string_2[] = "AT+IFC=1,0\r\n";
#else
Pchar gsm_init_string_2[] = "AT+GOI\r\n";
#endif
Pchar gsm_init_string_3[] = "AT+IPR=9600\r\n";
Pchar gsm_init_string_4[] = "AT+CSQ\r\n";
Pchar gsm_init_string_5[] = "AT+CREG=2\r\n";
Pchar gsm_init_string_6[] = "AT+CGREG=2\r\n";
PROGMEM const char *gsm_init_string_table[] = { gsm_init_string_0, gsm_init_string_1, gsm_init_string_2,
                                                gsm_init_string_3, gsm_init_string_4, gsm_init_string_5,
                                                gsm_init_string_6 };

#define GPRS_INIT_ATTACH_CMD 9
#define GPRS_INIT_LEN 12
Pchar gprs_init_string_0[] = "AT+CGACT?\r\n";
Pchar gprs_init_string_1[] = "AT+CGREG?\r\n";
Pchar gprs_init_string_2[] = "AT+CIPSHUT\r\n"; //+CGATT=1\r\n"; //+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
Pchar gprs_init_string_3[] = "AT+CIPMODE=0\r\n"; //AT+CDNSCFG=\"8.8.8.8\",\"8.8.8.4\"\r\n";
Pchar gprs_init_string_4[] = "AT+CGATT=1\r\n";
Pchar gprs_init_string_5[] = "AT+CGDCONT=1,\"IP\",\"internet\"\r\n";
//Pchar gprs_init_string_5[] = "AT+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
Pchar gprs_init_string_6[] = "AT+CLPORT=\"TCP\",\"2020\"\r\n";
Pchar gprs_init_string_7[] = "AT+CSTT=\"internet\",\"\",\"\"\r\n";
Pchar gprs_init_string_8[] = "AT+CIPSRIP=1\r\n";
Pchar gprs_init_string_9[] = "AT+CIICR\r\n";
Pchar gprs_init_string_10[] = "AT+CIFSR\r\n";
Pchar gprs_init_string_11[] = "AT+CIPSEND?\r\n"; //CDNSCFG=\"8.8.8.8\",\"8.8.8.4\"\r\n";

PROGMEM const char *gprs_init_string_table[] = { gprs_init_string_0, gprs_init_string_1, gprs_init_string_2,
                                                 gprs_init_string_3, gprs_init_string_4, gprs_init_string_5,
                                                 gprs_init_string_6, gprs_init_string_7, gprs_init_string_8,
                                                 gprs_init_string_9, gprs_init_string_10, gprs_init_string_11 
};

prog_char gsm_string_0[] PROGMEM = "\r\nATE0\r\n";
prog_char gsm_string_1[] PROGMEM = "AT+GOI\r\n";
prog_char gsm_string_2[] PROGMEM = "AT+CREG=2\r\n";
prog_char gsm_string_3[] PROGMEM = "AT+CGREG=2\r\n";
prog_char gsm_string_4[] PROGMEM = "AT+CGREG?\r\n";
prog_char gsm_string_5[] PROGMEM = "AT+CGATT?\r\n";
prog_char gsm_string_6[] PROGMEM = "AT+CGATT=1\r\n";
//prog_char gsm_string_7[] PROGMEM = "AT+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
prog_char gsm_string_7[] PROGMEM = "AT+CGDCONT=1,\"IP\",\"internet.bibob.dk\"\r\n";
prog_char gsm_string_8[] PROGMEM = "AT+CDNSCFG=\"8.8.8.8\",\"8.8.8.4\"\r\n";
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
prog_char gsm_string_19[] PROGMEM = "AT+CGACT=1,1\r\n";
PROGMEM const char *gsm_string_table[] = { gsm_string_0, gsm_string_1, gsm_string_2,
                                           gsm_string_3, gsm_string_4, gsm_string_5,
                                           gsm_string_6, gsm_string_7, gsm_string_8,
                                           gsm_string_9, gsm_string_10, gsm_string_11,
                                           gsm_string_12, gsm_string_13, gsm_string_14,
                                           gsm_string_15, gsm_string_16, gsm_string_17,
                                           gsm_string_18, gsm_string_19 };

#define GPRSS_LEN 13
Pchar gprs_state_0[] = "IP INITIAL";
Pchar gprs_state_1[] = "IP START";
Pchar gprs_state_2[] = "IP CONFIG";
Pchar gprs_state_3[] = "IP GPRSACT";
Pchar gprs_state_4[] = "IP STATUS";
Pchar gprs_state_5[] = "TCP CONNECTING";
Pchar gprs_state_6[] = "UDP CONNECTING";
Pchar gprs_state_7[] = "CONNECT OK";
Pchar gprs_state_8[] = "TCP CLOSING";
Pchar gprs_state_9[] = "UDP CLOSING"; 
Pchar gprs_state_10[] = "TCP CLOSED";
Pchar gprs_state_11[] = "UDP CLOSED";
Pchar gprs_state_12[] = "PDP DEACT";


PROGMEM const char *gprs_state_table[] = { gprs_state_0, gprs_state_1, gprs_state_2,
                                           gprs_state_3, gprs_state_4, gprs_state_5,
                                           gprs_state_6, gprs_state_7, gprs_state_8,
                                           gprs_state_9, gprs_state_10, gprs_state_11,
                                           gprs_state_12 };
#define PRINTDBG(a, v) if (a) { \
                        Serial.print(v); \
                        } \

NewSoftSerial _gsmserial(GSM_RX, GSM_TX);

DLGSM::DLGSM()
{
        for(uint8_t k=0;k<5;k++) {
                _gsm_lac[k] = 0;
                _gsm_ci[k] =  0;
        }
        _gsm_callback = NULL;
}
                
void DLGSM::debug(uint8_t v) {
	_DEBUG = v;
}

uint8_t DLGSM::CONN_get_flag(uint8_t f) {
	return (_c.flags & f);
}

void DLGSM::CONN_set_flag(uint8_t f, uint8_t v) {
	if (v)
		_c.flags = _c.flags | f;
	else
		_c.flags = _c.flags & ~f;
	if (_DEBUG) {
		Serial.print("FLAGS: ");
		Serial.println(_c.flags, DEC);
	}
}

void DLGSM::init(char *buff, int buffsize, uint8_t tout = 10) {
        _gsm_buff = buff; // Work area
        _gsm_buffsize = buffsize; // Size of area
        _gsm_tout = tout; // Timeout*100ms
	_gsmserial.begin(GSM_BAUD);
}

uint8_t DLGSM::GSM_init() {
	for(int k=0;k<GSM_INIT_LEN;k++) {
#ifdef WATCHDOG
		wdt_reset();
#endif
		get_from_flash(&(gsm_init_string_table[k]), _gsm_buff);
		GSM_send(_gsm_buff);
		_gsm_ret = GSM_process("OK");	
		if (!_gsm_ret)
			return 0;
	}
	return 1;
}

uint8_t DLGSM::GSM_process(char *check) {
	int i = 0, a = 0, nr = _gsm_tout, bs = 0;
	uint8_t ret = 0;
	do {
#ifdef WATCHDOG
		wdt_reset();
#endif
		a = _gsmserial.available();
		if (a) {
			nr = _gsm_tout;
			bs = GSM_recvline(_gsm_buff, _gsm_buffsize);
			if (_gsm_callback) {
				ret += _gsm_callback(_gsm_buff, bs);
			}
			if (strcmp_P(_gsm_buff, PSTR("+CGREG:")) == 0) {
				strncpy(_gsm_lac, _gsm_buff+13, 4);
       				strncpy(_gsm_ci, _gsm_buff+20, 4);
    			}
			if (strcmp_P(_gsm_buff, PSTR("+CREG:")) == 0) {
				strncpy(_gsm_lac, _gsm_buff+12, 4);
				strncpy(_gsm_ci, _gsm_buff+19, 4);
			}
			if (strcmp_P(_gsm_buff, PSTR("CLOSE OK")) == 0) { // CLOSED
				CONN_set_flag(CONN_CONNECTED, 0);
			}
			if (strcmp_P(_gsm_buff, PSTR("CONNECT OK")) == 0) {
				CONN_set_flag(CONN_CONNECTED, 1);
			}
			if (check != NULL) {
				if (strstr(_gsm_buff, check) != 0) {
					return 1;
				}
			}   
      			i++;
    		} else {
      			nr--;
      			delay(100);
    		}
  	} while (a || nr > 0);
  	return ret;
}

uint8_t DLGSM::GSM_process(char *check, uint8_t tout) {
	uint8_t otout = _gsm_tout;
	uint8_t r = 0;
	GSM_set_timeout(tout);
	r = GSM_process(check);
	GSM_set_timeout(otout);
	return r;
}

uint8_t DLGSM::GSM_fast_read(char *until, FUN_callback fun) {
	int b = 0, cdown = 100;
	while (1) {
#ifdef GSM_SW_FLOW
		GSM_Xon();
#endif
		b = GSM_recvline_fast(_gsm_buff, _gsm_buffsize);
#ifdef GSM_SW_FLOW
		GSM_Xoff();
#endif
		if (b) {
			cdown = 100;	
			if (fun)
				fun(_gsm_buff, b);
			if (_DEBUG)		
				Serial.print(_gsm_buff);
			if (strstr(_gsm_buff, until) != 0)
				break;
		} else {
			cdown--;
			if (cdown <= 0)
				return 0;
			delay(10);
		}

	}	
	return 1;
}

void DLGSM::GSM_send(char v) {
	_gsmserial.print(v);
	PRINTDBG(_DEBUG, v);	
}

void DLGSM::GSM_send(int v) {
	_gsmserial.print(v);
	PRINTDBG(_DEBUG, v);
}

void DLGSM::GSM_send(char *v) {
	_gsmserial.print(v);
	_gsmserial.flush();
        PRINTDBG(_DEBUG, v);
}

void DLGSM::GSM_send(unsigned long v) {
	_gsmserial.print(v);
        PRINTDBG(_DEBUG, v);
}

void DLGSM::GSM_send(float v) {
	_gsmserial.print(v);
        PRINTDBG(_DEBUG, v);
}

void DLGSM::GSM_send(char *v, int len) {
        PRINTDBG(_DEBUG, v);
}

void DLGSM::GSM_set_timeout(int tout) {
	_gsm_tout = tout;
}

void DLGSM::GSM_Xon() {
	GSM_send(0x11);
}

void DLGSM::GSM_Xoff() {
	GSM_send(0x13);
}

int DLGSM::GSM_recvline_fast(char *ptr, int len) {
	int a = 0, i = 0, k = 0;
	char cchar = 0;
	while (i < len) {
		a = _gsmserial.available();
		if (a > 0) {
			for(k=0; cchar != '\n' && k<a && i < len;k++) {
				cchar = _gsmserial.read();
				ptr[i] = cchar;
				i++;
			}
			if (cchar == '\n') {
				ptr[i] = 0;
				break;
			}
		} else {
			ptr[i] = 0;
			break;
		}
	}
	return i;
}


int DLGSM::GSM_recvline(char *ptr, int len) {
	char curchar[2];
	int i = 0, a=0, c = 0;
	// Read the first 2 bytes (should be the beginning of a reply)
	for(i=0;i<2;i++) {
		if (_gsmserial.available())
    			curchar[i] = _gsmserial.read();
		else
			break;
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
	curchar[0] = 0;
	while (i < len) {
		a = _gsmserial.available();
		if (a > 0) {
			for(c=0;c<a && curchar[0] != '\n' && i < len;c++){
				curchar[0] = _gsmserial.read();
				ptr[i] = curchar[0];
				i++;
			}
			if (curchar[0] == '\n') {
				ptr[i] = 0;
				break;
			}
		} else {
			ptr[i] = 0;
      			break;
    		}
 	}
 	ptr[i] = 0;
	if (_DEBUG) {
		Serial.print(i);
		Serial.print(": ");
		Serial.println(ptr);
	}
	return i;
}

void DLGSM::GSM_request_net_status() {
	get_from_flash(&(gsm_string_table[4]), _gsm_buff);
	GSM_send(_gsm_buff);
	_gsm_ret = GSM_process("OK");
}

void DLGSM::GSM_get_local_time() {
}

void DLGSM::GSM_set_callback(FUN_callback fun) {
	_gsm_callback = fun;
}
                
uint8_t DLGSM::GPRS_init() {
	for(uint8_t k=0;k<GPRS_INIT_LEN;k++) {
#ifdef WATCHDOG
		wdt_reset();
#endif
		if (k == GPRS_INIT_ATTACH_CMD)
			GSM_set_timeout(50);
		get_from_flash(&(gprs_init_string_table[k]), _gsm_buff);
		GSM_send(_gsm_buff);
		_gsm_ret = GSM_process("OK");
		if (k == GPRS_INIT_ATTACH_CMD)
			GSM_set_timeout(25);
		//if (!_gsm_ret)
		//	return 0;
	}
	GSM_set_timeout(10);
	return 1;
}



uint8_t DLGSM::GPRS_connect(char *server, short port, bool proto) {
	if (CONN_get_flag(CONN_CONNECTED)) {
		if (CONN_get_flag(CONN_SENDING))
			GPRS_send_end();
		GPRS_close();
	}

	if (proto)
		get_from_flash(&(gsm_string_table[13]), _gsm_buff);
	else
		get_from_flash(&(gsm_string_table[14]), _gsm_buff);
	GSM_send(_gsm_buff);
	GSM_send(server);
	GSM_send("\",\"");
	GSM_send(port);
	GSM_send("\"\r\n");
	uint8_t s = GSM_process("CONNECT OK", 30);
	for(int wait=GPRS_CONN_TIMEOUT;wait>0;wait--) {
#ifdef WATCHDOG
		wdt_reset();
#endif
		s = GPRS_check_conn_state();
		if (s == GPRSS_CONNECT_OK)
			break;
	}
	return s;
}
            
uint8_t DLGSM::GPRS_send_start() {
	uint8_t nr=5;
	do {
		get_from_flash(&(gsm_string_table[15]), _gsm_buff); // Send AT+CIPSEND
		GSM_send(_gsm_buff);
		delay(100);
		nr--;
	} while (GSM_process(">",20) == 0 && nr > 0);
	if (nr <= 0)
		return 0;
	else
		CONN_set_flag(CONN_SENDING, 1);
	return 1;
}

void DLGSM::GPRS_send(char *data) {
	GSM_send(data);
}

void DLGSM::GPRS_send_raw(char *data, int len) {
	GSM_send(data, len);
}

void DLGSM::GPRS_send(float n) {
	GSM_send(n);
}

void DLGSM::GPRS_send(unsigned long n) {
	GSM_send(n);
}
void DLGSM::GPRS_send(int n) {
	GSM_send(n);
}

uint8_t DLGSM::GPRS_send_end() {
	uint8_t r = 0;
	if (CONN_get_flag(CONN_SENDING)) {
		_gsmserial.print(26, BYTE);
		GSM_send(0x1a);
		r = GSM_process("SEND OK", 30);
		if (r) CONN_set_flag(CONN_SENDING, 0);
	}
	return r;
}

uint8_t DLGSM::GPRS_close() {
	int r = 0;
	int s = GPRS_check_conn_state();
	if (s == GPRSS_CONNECT_OK) {
		get_from_flash(&(gsm_string_table[16]), _gsm_buff); // Send AT+CIPCLOSE
		GSM_send(_gsm_buff);
		r = GSM_process("CLOSE", 30);
		return r;
	} else
		return 1;
}

int8_t DLGSM::GPRS_check_conn_state() {
	uint8_t k = 0;
	char d[15];
#ifdef WATCHDOG
	wdt_reset();
#endif
	get_from_flash(&(gsm_string_table[17]), _gsm_buff);
	GSM_send(_gsm_buff);
	GSM_process(NULL);
	if (strcmp_P(_gsm_buff, PSTR("STATE:")) >= 0) {
		for(k=0; k < GPRSS_LEN; k++) {
			if (strcmp_flash(_gsm_buff+7, &(gprs_state_table[k]), d) == 0) {
				switch(k) {
					case GPRSS_IP_INITIAL:
					case GPRSS_IP_START:
					case GPRSS_IP_CONFIG: // Reinitialize GPRS
						GSM_init();
						GPRS_init();
						break; 
					case GPRSS_IP_GPRSACT: // Just need to query the local IP...
						get_from_flash(&(gprs_init_string_table[10]), _gsm_buff);
						GSM_send(_gsm_buff);
						GSM_process(NULL, 25);
						break;
					case GPRSS_CONNECT_OK:
						CONN_set_flag(CONN_CONNECTED, 1);
						break;
					case GPRSS_TCP_CLOSED:
					case GPRSS_UDP_CLOSED:
						CONN_set_flag(CONN_CONNECTED, 0);
						break; 
					case GPRSS_PDP_DEACT: // Reinitialize GPRS
						GSM_init();
						GPRS_init();
						break;			
					default:
						break;
				};				
				return k;
			}
		}
	}
	return -1;
}

