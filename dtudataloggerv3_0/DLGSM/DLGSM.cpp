//#include "WProgram.h"
#include <Arduino.h>
#include "DLGSM.h"

#define Pchar prog_char PROGMEM

#define GSM_INIT_LEN 6
Pchar gsm_init_string_0[] = "AT\r\n";
Pchar gsm_init_string_1[] = "ATE0\r\n";
//#ifdef GSM_SW_FLOW
//Pchar gsm_init_string_2[] = "AT+IFC=1,0\r\n";
//#else
//Pchar gsm_init_string_2[] = "AT+GOI\r\n";
//#endif
//Pchar gsm_init_string_3[] = "AT+IPR=9600\r\n";
//Pchar gsm_init_string_4[] = "AT+CSQ\r\n";
Pchar gsm_init_string_2[] = "AT+CREG=2\r\n";
Pchar gsm_init_string_3[] = "AT+CGREG=2\r\n";
Pchar gsm_init_string_4[] = "AT+IPR=57600\r\n";
Pchar gsm_init_string_5[] = "AT+CNMI=2,1,0,1,0\r\n";
PROGMEM const char *gsm_init_string_table[] = { gsm_init_string_0, gsm_init_string_1, gsm_init_string_2,
                                                gsm_init_string_3, gsm_init_string_4, gsm_init_string_5 }; ///,

#define GPRS_INIT_ATTACH_CMD 9
#define GPRS_INIT_LEN 10
#define GPRS_INIT_NONTWR_LEN 2
//Pchar gprs_init_string_0[] = "AT+CGACT?\r\n";
Pchar gprs_init_string_0[] = "AT+CGREG?\r\n";
Pchar gprs_init_string_1[] = "AT+CIPSHUT\r\n"; //+CGATT=1\r\n"; //+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
//Pchar gprs_init_string_2[] = "AT+CIPMODE=0\r\n"; //AT+CDNSCFG=\"8.8.8.8\",\"8.8.8.4\"\r\n";
Pchar gprs_init_string_2[] = "AT+CDNSCFG=\"8.8.8.8\",\"8.8.8.4\"\r\n";
Pchar gprs_init_string_3[] = "AT+CGATT=1\r\n";
Pchar gprs_init_string_4[] = "AT+CGDCONT=1,\"IP\",\"internet\"\r\n";
//Pchar gprs_init_string_5[] = "AT+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
Pchar gprs_init_string_5[] = "AT+CLPORT=\"TCP\",\"2020\"\r\n";
Pchar gprs_init_string_6[] = "AT+CSTT=\"internet\",\"\",\"\"\r\n";
Pchar gprs_init_string_7[] = "AT+CIPSRIP=1\r\n";
Pchar gprs_init_string_8[] = "AT+CIICR\r\n";
Pchar gprs_init_string_9[] = "AT+CIFSR\r\n";

PROGMEM const char *gprs_init_string_table[] = { gprs_init_string_0, gprs_init_string_1, gprs_init_string_2,
                                                 gprs_init_string_3, gprs_init_string_4, gprs_init_string_5,
                                                 gprs_init_string_6, gprs_init_string_7, gprs_init_string_8,
                                                 gprs_init_string_9 };

prog_char gsm_string_0[] PROGMEM = "AT+CGREG?\r\n";
prog_char gsm_string_1[] PROGMEM = "AT+CIPSTART=\"TCP\",\"";
prog_char gsm_string_2[] PROGMEM = "AT+CIPSTART=\"UDP\",\"";
prog_char gsm_string_3[] PROGMEM = "AT+CIPSEND\r\n";
prog_char gsm_string_4[] PROGMEM = "AT+CIPCLOSE\r\n";
prog_char gsm_string_5[] PROGMEM = "AT+CIPSTATUS\r\n";
prog_char gsm_string_6[] PROGMEM = "AT+CIPSEND?\r\n";
PROGMEM const char *gsm_string_table[] = { gsm_string_0, gsm_string_1, gsm_string_2,
                                           gsm_string_3, gsm_string_4, gsm_string_5, gsm_string_6 };

prog_char sms_string_0[] PROGMEM = "AT+CMGF=1\r\n";
prog_char sms_string_1[] PROGMEM = "AT+CSCS=\"IRA\"\r\n";
prog_char sms_string_2[] PROGMEM = "AT+CMGS=\"";
PROGMEM const char *sms_string_table[] = {  sms_string_0, sms_string_1, sms_string_2 };

#define GPRSS_LEN 13
Pchar gprs_state_0[] = "IP IN"; //ITIAL";
Pchar gprs_state_1[] = "IP STAR"; //T";
Pchar gprs_state_2[] = "IP CO"; //NFIG";
Pchar gprs_state_3[] = "IP GP"; //RSACT";
Pchar gprs_state_4[] = "IP STAT"; //US";
Pchar gprs_state_5[] = "TCP CO"; //NNECTING";
Pchar gprs_state_6[] = "UDP CO"; //NNECTING";
Pchar gprs_state_7[] = "CONN"; //ECT OK";
Pchar gprs_state_8[] = "TCP CL"; //OSING";
Pchar gprs_state_9[] = "UDP CL"; //OSING"; 
Pchar gprs_state_10[] = "TCP CLOSED";
Pchar gprs_state_11[] = "UDP CLOSED";
Pchar gprs_state_12[] = "PDP";// DEACT";

static int gsm_pt_i;

PROGMEM const char *gprs_state_table[] = { gprs_state_0, gprs_state_1, gprs_state_2,
                                           gprs_state_3, gprs_state_4, gprs_state_5,
                                           gprs_state_6, gprs_state_7, gprs_state_8,
                                           gprs_state_9, gprs_state_10, gprs_state_11,
                                           gprs_state_12 };
#define PRINTDBG(a, v) if (a) { \
                        Serial.print(v); \
                        } \

#define DEBUG 1

#ifdef HARDWARE_SERIAL
#define _gsmserial Serial1
#else
SoftwareSerial _gsmserial(GSM_RX, GSM_TX);
#endif

DLGSM::DLGSM()
{
	pinMode(GSM_PWR, INPUT);
        for(uint8_t k=0;k<5;k++) {
                _gsm_lac[k] = 0;
                _gsm_ci[k] =  0;
        }
        _gsm_callback = NULL;
	_gsm_wline = 0;
	_sendsize = 0;
}
        
#ifdef USE_PT
int DLGSM::PT_recvline(struct pt *pt, char *ret, char *ptr, int len, int tout, char process) {
        int a = 0, k = 0;
        char cchar = 0;
	static uint32_t ts = 0;
	PT_BEGIN(pt);
	gsm_pt_i = 0;

        while (gsm_pt_i < len) {
		PT_WAIT_UNTIL(pt, _gsmserial.available() > 0 || (millis() - ts) > tout);
        	ts = millis();
	        a = _gsmserial.available();
                if (a > 0) { 
			for(k=0; cchar != '\n' && k<a && gsm_pt_i < len;k++) {
                		cchar = _gsmserial.read();
                                ptr[gsm_pt_i] = cchar;
                                gsm_pt_i++;
                        }
                        if (cchar == '\n') {
                                ptr[gsm_pt_i] = 0;
				if (gsm_pt_i > 2) {
					Serial.print("Line: ");
					Serial.println(ptr);
					if (process)
						GSM_process_line(NULL);
				}
				*ret = gsm_pt_i;
                                PT_EXIT(pt);
                        }
                } else if (_gsm_wline) {
			ptr[gsm_pt_i] = 0;
			Serial.print("Line: ");
			Serial.println(ptr);
			*ret = gsm_pt_i;
			PT_EXIT(pt);
		}
        }
	PT_END(pt);
}

int DLGSM::PT_recv(struct pt *pt, char *ret, char *conf, int tout) {
	static uint32_t ts;
	static struct pt linerecv_pt;
	
	PT_BEGIN(pt);
	*ret = 0;
	PT_WAIT_UNTIL(pt, _gsmserial.available() || (millis() - ts) > 1000);
	ts = millis();

	if (!_gsmserial.available()) {
		PT_RESTART(pt);
	}
	
        while (_gsmserial.available()) {
                PT_WAIT_THREAD(pt, PT_recvline(&linerecv_pt, ret, _gsm_buff, _gsm_buffsize, tout, 1));
                if (conf != NULL && strstr(_gsm_buff, conf) != NULL)
                        *ret = 1;
                else if (conf != NULL)
                        *ret = 0;
        }
	PT_END(pt);
}

int DLGSM::PT_send_recv(struct pt *pt, char *ret, char *cmd, int tout) {
	static uint32_t ts, startts;
	static struct pt linerecv_pt;
	static char gotsmtg = 0;
	PT_BEGIN(pt);
	*ret = 0;
	gotsmtg = 0;
	GSM_send(cmd);
	PT_WAIT_UNTIL(pt, _gsmserial.available() || (millis() - ts) > 1000);
	ts = millis();

	if (!_gsmserial.available()) {
		PT_RESTART(pt);
	}
	startts = millis();
	_gsm_wline = 1;
	while (_gsmserial.available()  || (millis() - startts) < 5000) {
		PT_WAIT_THREAD(pt, PT_recvline(&linerecv_pt, ret, _gsm_buff, _gsm_buffsize, tout, 1));
		if (*ret > gotsmtg)
			gotsmtg = *ret;
		ts = millis();
	}
	if (*ret == 0)
		*ret = gotsmtg;
	_gsm_wline = 0;
	PT_END(pt);
}

int DLGSM::PT_send_recv_confirm(struct pt *pt, char *ret, char *cmd, char *conf, int tout) {
	static uint32_t ts,startts;
	static struct pt linerecv_pt;
	static char cdown = 0;
	PT_BEGIN(pt);
	*ret = 0;
	if (cdown == 0)
		cdown = 5;
	GSM_send(cmd);
	PT_WAIT_UNTIL(pt, _gsmserial.available() || (millis() - ts) > tout);
	ts = millis();
	
	if (!_gsmserial.available()) {
		PT_RESTART(pt);
	}
	startts = millis();

	while (_gsmserial.available() || (millis() - startts) < 10000) { 
		PT_WAIT_THREAD(pt, PT_recvline(&linerecv_pt, ret, _gsm_buff, _gsm_buffsize, tout, 1));
		ts = millis();
		if (conf != NULL && strstr(_gsm_buff, conf) != NULL) {
			*ret = 1;
			PT_EXIT(pt);
		} else if (conf != NULL && (strstr(_gsm_buff, "ERROR") != NULL || strstr(_gsm_buff, "FAIL") != NULL)) {
			*ret = 2;
			PT_EXIT(pt);
		} else if (conf != NULL)
			*ret = 0;
	}
	PT_END(pt);
}

int DLGSM::PT_GSM_init(struct pt *pt, char *ret) {
	static uint32_t ts=0;
	static struct pt child_pt;
	static char iret = 0, k;
	PT_BEGIN(pt);
	k = 0;
        //pwr_on();
	PT_WAIT_UNTIL(pt, millis()-ts > 1000); // Wait until module inits
	ts = millis();

	while (k < GSM_INIT_LEN) {
		iret = 0;
                get_from_flash(&(gsm_init_string_table[k]), _gsm_buff);
		PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &iret, _gsm_buff, "OK", 1000));
		Serial.println("ye");
		if (iret > 0)
			k++;
        }
	*ret = 1;
	PT_END(pt);
}

int DLGSM::PT_GPRS_init(struct pt *pt, char *ret) {
        static uint8_t len;
	static struct pt child_pt;
	static char k;
        PT_BEGIN(pt);
	//if (!CONN_get_flag(CONN_NETWORK))
        //        len = GPRS_INIT_NONTWR_LEN;
	//else
	len = GPRS_INIT_LEN;
	k = 0;
	while (k < len) { 	
                if (k == GPRS_INIT_ATTACH_CMD)
                        GSM_set_timeout(50);
        	*ret = 0;
	        get_from_flash(&(gprs_init_string_table[k]), _gsm_buff);
		if (k != (len-1)) {
			Serial.println("need confirm");
                	PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, ret, _gsm_buff, "OK", 1000));
		} else {
			Serial.println("just waiting");
			PT_WAIT_THREAD(pt, PT_send_recv(&child_pt, ret, _gsm_buff, 3000));
			Serial.print("Got ret:");
			Serial.println(_gsm_buff);
		}
		Serial.println("ye2");
                if (*ret > 0)
			k++;
		if (k == GPRS_INIT_ATTACH_CMD)
                        GSM_set_timeout(25);
        }
        GSM_set_timeout(10);
	PT_END(pt);
}

int DLGSM::PT_GSM_event_handler(struct pt *pt, char *ret) {
        char i = 0;
	static struct pt child_pt;
	PT_BEGIN(pt);	

	PT_YIELD_UNTIL(pt, _gsmserial.available() > 1);
	
       	while (_gsmserial.available()) {
        	PT_WAIT_THREAD(pt, PT_recvline(&child_pt, ret, _gsm_buff, _gsm_buffsize, 1000, 1));
        	if (*ret > 0) {
                	// Call arriving, hang up?
                	if (_gsm_buff[0] == 'R') {
				sprintf(_gsm_buff, "ATH\r\n");
				PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, ret, _gsm_buff, "OK", 5000));
				*ret = GSM_EVENT_STATUS_REQ;
				PT_EXIT(pt);
                	} else {
				GSM_process_line(NULL);
			}
        	}
	}
	PT_END(pt);
}


int DLGSM::PT_GPRS_check_conn_state(struct pt *pt, char *ret) {
	static struct pt child_pt;
        static uint8_t k;
	char iret;
        char d[15];

	PT_BEGIN(pt);
        get_from_flash(&(gsm_string_table[5]), _gsm_buff);
	*ret = 0;
	k = 0;
        PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, ret, _gsm_buff, "STATE:", 20000));
	Serial.print("Last line: ");
	Serial.println(_gsm_buff);

        if (strcmp_P(_gsm_buff, PSTR("STATE:")) >= 0) {
		Serial.println("got state");
		while (k < GPRSS_LEN) {
			Serial.print(" doing ");
			Serial.println(k, DEC);
                        if (strcmp_flash(_gsm_buff+7, &(gprs_state_table[k]), d) == 0) {
				*ret = k;
				Serial.println("detected");
                                if (k == GPRSS_IP_INITIAL ||
                                    k == GPRSS_IP_START ||
                                    k == GPRSS_IP_CONFIG) { // Reinitialize GPRS
						PT_WAIT_THREAD(pt, PT_GSM_init(&child_pt, &iret));
						PT_WAIT_THREAD(pt, PT_GPRS_init(&child_pt, &iret));              
                                } else if (k == GPRSS_IP_GPRSACT) { // Just need to query the local IP...
                                	get_from_flash(&(gprs_init_string_table[9]), _gsm_buff);
                                      	PT_WAIT_THREAD(pt, PT_send_recv(&child_pt, &iret, _gsm_buff, 1000));
                                } else if (k == GPRSS_CONNECT_OK) {
                                	CONN_set_flag(CONN_CONNECTED, 1);
                                } else if (k == GPRSS_TCP_CLOSED ||
                                           k == GPRSS_UDP_CLOSED) {
                                	CONN_set_flag(CONN_CONNECTED, 0);
                                } else if (k == GPRSS_PDP_DEACT) { // Reinitialize GPRS
                                	PT_WAIT_THREAD(pt, PT_GSM_init(&child_pt, &iret));
					PT_WAIT_THREAD(pt, PT_GPRS_init(&child_pt, &iret));
				}
                                *ret = k;
				PT_EXIT(pt);
                        }
			k++;
                }
        } else {
		PT_RESTART(pt);
	}
        *ret = -1;
	PT_END(pt);
}

int DLGSM::PT_SMS_send(struct pt *pt, char *ret, char *nr, char *text, int len) {
	static struct pt child_pt;
        uint8_t c = 0, r = 0;

	PT_BEGIN(pt);

	while (c == 0) {
                get_from_flash(&(sms_string_table[0]), _gsm_buff);
                PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, ret, _gsm_buff, "OK", 1000));
        	c = *ret;
	}

        get_from_flash(&(sms_string_table[1]), _gsm_buff);
	PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, ret, _gsm_buff, "OK", 3000));

	_gsm_wline = 1;
	while (c == 0) {
                get_from_flash(&(sms_string_table[2]), _gsm_buff);
                strcat(_gsm_buff, nr);
		strcat(_gsm_buff, "\"\r\n");
		PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, ret, _gsm_buff, ">", 3000));
		Serial.println("hi");
        	c = *ret;
	}
	_gsm_wline = 0;
        GSM_send(text, len);
        
	PT_WAIT_THREAD(pt, PT_SMS_send_end(&child_pt));
	PT_END(pt);
}

int DLGSM::PT_SMS_send_end(struct pt *pt) {
	static struct pt child_pt;
        uint8_t r = 0;
	char iret;
	PT_BEGIN(pt);
        
	_gsm_buff[0] = 0x1a;
	_gsm_buff[1] = '\0';
	PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &iret, _gsm_buff, "OK", 3000));
      
	PT_END(pt);
}


int DLGSM::PT_GPRS_connect(struct pt *pt, char *ret, char *server, short port, bool proto) {
	char r = 0;
	char shortbuff[100];
	static struct pt child_pt;
	PT_BEGIN(pt);

        if (CONN_get_flag(CONN_CONNECTED)) {
                if (CONN_get_flag(CONN_SENDING))
                        PT_WAIT_THREAD(pt, PT_GPRS_send_end(&child_pt, &r));
                PT_WAIT_THREAD(pt, PT_GPRS_close(&child_pt, &r));
        }

        if (proto)
                get_from_flash(&(gsm_string_table[1]), _gsm_buff);
        else
                get_from_flash(&(gsm_string_table[2]), _gsm_buff);
	
	sprintf(shortbuff, "%s\",\"%d\"\r\n", server, port);
        strcat(_gsm_buff, shortbuff);

	PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &r, _gsm_buff, "CONNECT", 3000));

	PT_WAIT_THREAD(pt, PT_GPRS_check_conn_state(&child_pt, &r));
	if (r == GPRSS_CONNECT_OK)
		*ret = 1;
	else
		*ret = 0;

	PT_END(pt);
}

int DLGSM::PT_GPRS_send_start(struct pt *pt, char *ret) {
        char r;
	static struct pt child_pt;
	PT_BEGIN(pt);

        get_from_flash(&(gsm_string_table[6]), _gsm_buff); // Send AT+CIPSEND? to get 
        PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &r, _gsm_buff, "+", 3000));
                
	get_from_flash(&(gsm_string_table[3]), _gsm_buff); // Send AT+CIPSEND
	_gsm_wline = 1; // Disable new line character expectancy	
	PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &r, _gsm_buff, ">", 3000));
	_gsm_wline = 0; // Enable new line expectancy   
        if (r > 0)        
		CONN_set_flag(CONN_SENDING, 1);
	*ret = r; 
	PT_END(pt);
}

int DLGSM::PT_GPRS_send_end(struct pt *pt, char *ret) {
        char r = 0;
	static struct pt child_pt;
	PT_BEGIN(pt);
	r = 0;
        if (CONN_get_flag(CONN_SENDING)) {
                _gsm_buff[0] = 0x1a;
		_gsm_buff[1] = '\0';
	        PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &r, _gsm_buff, "OK", 3000));
		if (r) CONN_set_flag(CONN_SENDING, 0);
        }
	*ret = r;
	PT_END(pt);
}

int DLGSM::PT_GPRS_close(struct pt *pt, char *ret) {
        char r = 0;
	static struct pt child_pt;
	
	PT_BEGIN(pt);
	
	PT_WAIT_THREAD(pt, PT_GPRS_check_conn_state(&child_pt, &r));

        if (r == GPRSS_CONNECT_OK) {
                get_from_flash(&(gsm_string_table[4]), _gsm_buff); // Send AT+CIPCLOSE
	   	PT_WAIT_THREAD(pt, PT_send_recv_confirm(&child_pt, &r, _gsm_buff, "OK", 3000));
                *ret = r;
        } else
                *ret = 1;
	PT_END(pt);
}

#endif

        
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

void DLGSM::pwr_on() {
	if (!CONN_get_flag(CONN_PWR)) {
#ifdef USE_PT
#else
		delay(1000);
#endif
		pinMode(GSM_PWR, OUTPUT);
		digitalWrite(GSM_PWR, LOW);
#ifdef WATCHDOG
		wdt_reset();
#endif
#ifdef USE_PT
#else
		delay(1000);
#endif
		pinMode(GSM_PWR, INPUT);
#ifdef USE_PT
#else
		delay(1000);
#endif
		CONN_set_flag(CONN_PWR, 1);
	}
}

void DLGSM::pwr_off(uint8_t force) {
	if (CONN_get_flag(CONN_PWR) || force) {
#ifdef USE_PT
#else
		delay(1000);
#endif
		pinMode(GSM_PWR, OUTPUT);
        	digitalWrite(GSM_PWR, LOW);
#ifdef WATCHDOG
		wdt_reset();
#endif
#ifdef USE_PT
#else
     		delay(1000);
#endif
        	pinMode(GSM_PWR, INPUT);
#ifdef USE_PT
#else
		delay(1000);
#endif
		CONN_set_flag(CONN_PWR, 0);
		CONN_set_flag(CONN_NETWORK, 0);
		CONN_set_flag(CONN_SENDING, 0);
		CONN_set_flag(CONN_CONNECTED, 0);
	}
}

void DLGSM::pwr_off() {
	pwr_off(0);
}

#ifdef USE_PT
uint8_t DLGSM::wake_modem(struct pt *pt) {
	uint8_t s = 0;
	char ret = 0;
	static struct pt child_pt;
	PT_BEGIN(pt);
	while (s < GPRSS_IP_STATUS || s >= GPRSS_PDP_DEACT) {
		PT_WAIT_THREAD(pt, PT_GPRS_check_conn_state(&child_pt, &ret));
		s = ret;
	}
	PT_END(pt);
}
#else
uint8_t DLGSM::wake_modem() {
	uint8_t j = 0, s = 0;
        for(j=0;j<10;j++) {
                s = GPRS_check_conn_state();
                if (s >= GPRSS_IP_STATUS && s < GPRSS_PDP_DEACT) {
                	return 1;
                }
                delay(10);
        }
	return 0;
}
#endif

void DLGSM::init(char *buff, int buffsize, uint8_t tout = 10) {
        _gsm_buff = buff; // Work area
        _gsm_buffsize = buffsize; // Size of area
        _gsm_tout = tout; // Timeout*100ms
	_gsmserial.begin(GSM_BAUD);
}

char* DLGSM::GSM_get_lac() {
	return &_gsm_lac[0];
}

char* DLGSM::GSM_get_ci() {
	return &_gsm_ci[0];
}

uint8_t DLGSM::GSM_init() {
	//pwr_on();
	wdt_reset();
#ifndef USE_PT
	delay(2000); // Wait 3s for module to stabilize
#endif
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

uint8_t DLGSM::GSM_process_line(char *check) {
	char i = 0;
	char l = 0;
	char *tpos;
	if (_gsm_buff[0] == '+' && (_gsm_buff[2] == 'G' || _gsm_buff[2] == 'R')) { // +CGREG: 2,1,"ASDA","XCVB"
		tpos = strchr(_gsm_buff, ' ');
		if (tpos == NULL) return 0;

		i = tpos[1] - '0';
		tpos = strchr(_gsm_buff, ',');
		if (tpos && tpos[1] != '"')
			i = tpos[1] - '0';

		if (i == 1)
			CONN_set_flag(CONN_NETWORK,1);
		else if (i == 0 || (i > 1 && i < 6) )
			CONN_set_flag(CONN_NETWORK,0);
		
		tpos = strchr(_gsm_buff, '"');
		if (tpos != NULL) {
			memset(&_gsm_lac[0], 0, 5);
			memset(&_gsm_ci[0], 0, 5);
			strncpy(_gsm_lac, tpos+1, 4);
			strncpy(_gsm_ci, tpos+8, 4);
			_gsm_lac[5] = 0;
			_gsm_ci[5] = 0;
		}

	} else if (_gsm_buff[0] == '+' && _gsm_buff[4] == 'S') { // +CIPSEND?
	//} else if (strncmp_P(_gsm_buff, PSTR("+CIPSEND"),8) == 0) { // +CIPSEND?
		_sendsize = atoi(_gsm_buff+10);
		Serial.print("ss: ");
		Serial.println(_sendsize);
	} else if (_gsm_buff[0] == 'C' && _gsm_buff[8] == 'K') { // CLOSE OK
	//} else if (strncmp_P(_gsm_buff, PSTR("CLOSE OK"),8) == 0) { // CLOSE OK
		CONN_set_flag(CONN_CONNECTED, 0);
	} else if (_gsm_buff[0] == 'C' && _gsm_buff[5] == 'D') { // CLOSED 
	//} else if (strncmp_P(_gsm_buff, PSTR("CLOSED"),6) == 0) {
		CONN_set_flag(CONN_CONNECTED, 0);
	} else if (_gsm_buff[0] == 'S' && _gsm_buff[6] == 'K') { // SEND OK
	//} else if (strncmp_P(_gsm_buff, PSTR("SEND OK"),7) == 0) {
		CONN_set_flag(CONN_SENDING, 0);
	} else if (_gsm_buff[0] == 'C' && _gsm_buff[10] == 'K') { // CONNECT OK
		//} else if (strncmp_P(_gsm_buff, PSTR("CONNECT OK"),10) == 0) {
		CONN_set_flag(CONN_CONNECTED, 1);
	} else if (_gsm_buff[0] == 'A' && _gsm_buff[1] == 'L') { // ALREADY CONNECT
		CONN_set_flag(CONN_CONNECTED, 1);
	}
	if (check != NULL) {
		if (strstr(_gsm_buff, check) != 0) {
			return 1;
		}
	}
}

uint8_t DLGSM::GSM_process(char *check) {
	int a = 0, nr = _gsm_tout, bs = 0;
	uint8_t i = 0;
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
    			GSM_process_line(check);		
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
			delay(1);
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
        //PRINTDBG(_DEBUG, v);
	for(uint8_t i = 0; i < len; i++) {
		_gsmserial.write((uint8_t)v[i]);
//		if (_DEBUG)
//			Serial.print((byte)v[i]);
	}
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
	int i = 0, a=0, c = 0, tout=0;
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
	tout=100;
	while (i < len) {
		a = _gsmserial.available();
		if (a > 0) {
			tout=100;
			for(c=0;c<a && curchar[0] != '\n' && i < len;c++){
				curchar[0] = _gsmserial.read();
				ptr[i] = curchar[0];
				i++;
			}
			if (curchar[0] == '\n') {
				ptr[i] = 0;
				break;
			}
		} else if (tout > 0) {
			tout--;
			delay(1);
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
	get_from_flash(&(gsm_string_table[0]), _gsm_buff);
	GSM_send(_gsm_buff);
	_gsm_ret = GSM_process("OK");
}

void DLGSM::GSM_get_local_time() {
}

uint8_t DLGSM::SMS_send(char *nr, char *text, int len) {
	uint8_t c = 0, r = 0;
	for(c = 0; c<5;c++) {
		get_from_flash(&(sms_string_table[0]), _gsm_buff);
		GSM_send(_gsm_buff);
		r = GSM_process("OK", 30);
		if (r)
			break;
	}
	get_from_flash(&(sms_string_table[1]), _gsm_buff);
	GSM_send(_gsm_buff);
	GSM_process("OK", 30);

	for(c = 0; c<5; c++) {
		get_from_flash(&(sms_string_table[2]), _gsm_buff);
		GSM_send(_gsm_buff);
		GSM_send(nr);
		GSM_send("\"\r\n");
		r = GSM_process(">", 30);
		if (r)
			break;
	}
	
	GSM_send(text, len);
	SMS_send_end();	
}

uint8_t DLGSM::SMS_send_end() {
        uint8_t r = 0;
        //_gsmserial.print(26, BYTE);
        _gsmserial.write(26);
	GSM_send(0x1a);
        r = GSM_process("OK", 30);
        return r;
}

void DLGSM::GSM_set_callback(FUN_callback fun) {
	_gsm_callback = fun;
}
                
uint8_t DLGSM::GPRS_init() {
	uint8_t len = GPRS_INIT_LEN;
	if (!CONN_get_flag(CONN_NETWORK))
		len = GPRS_INIT_NONTWR_LEN;

	for(uint8_t k=0;k<len;k++) {
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
		get_from_flash(&(gsm_string_table[1]), _gsm_buff);
	else
		get_from_flash(&(gsm_string_table[2]), _gsm_buff);
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
           
uint16_t DLGSM::GPRS_send_get_size() {
	return _sendsize;
}
 
uint8_t DLGSM::GPRS_send_start() {
	uint8_t nr=5;
	do {
		get_from_flash(&(gsm_string_table[6]), _gsm_buff); // Send AT+CIPSEND? to get 
		GSM_send(_gsm_buff); // The buffer size
		GSM_process("+");
		get_from_flash(&(gsm_string_table[3]), _gsm_buff); // Send AT+CIPSEND
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
		//_gsmserial.print(26, BYTE);
		_gsmserial.write(26);
		GSM_send(0x1a);
		r = GSM_process("OK", 30);
		if (r) CONN_set_flag(CONN_SENDING, 0);
	}
	return r;
}

uint8_t DLGSM::GPRS_close() {
	int r = 0;
	int s = GPRS_check_conn_state();
	if (s == GPRSS_CONNECT_OK) {
		get_from_flash(&(gsm_string_table[4]), _gsm_buff); // Send AT+CIPCLOSE
		GSM_send(_gsm_buff);
		r = GSM_process("OK", 30);
		return r;
	} else
		return 1;
}

int8_t DLGSM::GPRS_check_conn_state() {
	uint8_t k = 0;
	char d[15];
	get_from_flash(&(gsm_string_table[5]), _gsm_buff);
	GSM_send(_gsm_buff);
	GSM_process("STATE:", 20);
	if (strcmp_P(_gsm_buff, PSTR("STATE:")) >= 0) {
		for(k=0; k < GPRSS_LEN; k++) {
			if (strcmp_flash(_gsm_buff+7, &(gprs_state_table[k]), d) == 0) {
				if (k == GPRSS_IP_INITIAL ||
				    k == GPRSS_IP_START || 
				    k == GPRSS_IP_CONFIG) { // Reinitialize GPRS
						GSM_init();
						GPRS_init();
				} else if (k == GPRSS_IP_GPRSACT) { // Just need to query the local IP...
						get_from_flash(&(gprs_init_string_table[9]), _gsm_buff);
						GSM_send(_gsm_buff);
						GSM_process(NULL, 25);
				} else if (k == GPRSS_CONNECT_OK) {
						CONN_set_flag(CONN_CONNECTED, 1);
				} else if (k == GPRSS_TCP_CLOSED ||
					   k == GPRSS_UDP_CLOSED) {
						CONN_set_flag(CONN_CONNECTED, 0);
				} else if (k == GPRSS_PDP_DEACT) { // Reinitialize GPRS
						GSM_init();
						GPRS_init();
				}
				return k;
			}
		}
	}
	return -1;
}

int8_t DLGSM::GSM_event_handler() {
	char i = 0;
	if (!_gsmserial.available()) return 0;

	i = GSM_recvline(_gsm_buff, _gsm_buffsize);
	if (i > 0) {
		// Call arriving, hang up?
		if (_gsm_buff[0] == 'R') {
			GSM_send("ATH\r\n");			
			return GSM_EVENT_STATUS_REQ;
		}
	} 
	return 0;
}

int8_t DLGSM::available() {
	return _gsmserial.available();
}
