#include <Arduino.h>
#include "DLHTTP.h"

#define Pchar prog_char PROGMEM

#define HTTP_HEADERS_LEN 1
Pchar header_string_0[] = "Connection: close\r\n";
//Pchar header_string_1[] = "Content-Type: application/x-www-form-urlencoded\r\n";
PROGMEM const char *header_string_table[] = { header_string_0 };

uint8_t backend_err = 255;

int HTTP_process_reply(char *line, int len) {
	long timestamp = 0;
	if (line[0] == 'T' && line[1] == 'S') { // Get the unix timestamp from server
		if (strlen(line+3) >= 10) {
			timestamp = atol(line+3);
			if (abs(now()-timestamp) > TIME_DELTA)
				setTime(timestamp);
		}
	} else if (line[0] == 'E' && line[1] == 'R') { // ERR code
		backend_err = atoi(line+4);
	}
}

DLHTTP::DLHTTP()
{
	_backend_err = &backend_err;
}

void DLHTTP::init(char *http_buff, DLGSM *ptr) {
	_gsm = ptr;
	_http_buff = http_buff;
	_sent = 0;
}

#ifdef USE_PT
int DLHTTP::PT_backend_start(struct pt *pt, char *ret, char *host, uint16_t port) {
	static struct pt child_pt;
	static int error_cnt = 0;
	static uint32_t ts;

	PT_BEGIN(pt);

	*_backend_err = 255;	
	PT_WAIT_THREAD(pt, _gsm->PT_GPRS_connect(&child_pt, ret, host, port, true));
	if (*ret != 1) {
		error_cnt++;
		if (error_cnt > 5) {
			PT_WAIT_THREAD(pt, _gsm->PT_restart(&child_pt, ret));
			error_cnt = 0;
		}
		PT_WAIT_UNTIL(pt, (millis() - ts) > 5000);
		ts = millis();
		PT_WAIT_THREAD(pt, _gsm->PT_GPRS_init(&child_pt, ret));
		PT_RESTART(pt);
	}
	*ret = 0;
	
	while (*ret != 1) {
		PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_start(&child_pt, ret));
		if (*ret == 2)
			PT_EXIT(pt);
	}
	error_cnt = 0;

	PT_END(pt);
}

int DLHTTP::PT_backend_end(struct pt *pt, char *ret) {
	static struct pt child_pt;
	PT_BEGIN(pt);
	PT_WAIT_THREAD(pt, _gsm->PT_GPRS_close(&child_pt, ret));
	PT_END(pt);
}

int DLHTTP::PT_GET(struct pt *pt, char *ret, char *url) {
	static struct pt child_pt;
        static char *host, *query_string;

	PT_BEGIN(pt);
        parse_url(url, &host, &query_string);

	PT_WAIT_THREAD(pt, PT_backend_start(&child_pt, ret, host, 80));
	if (*ret != 1) PT_EXIT(pt);

        get_from_flash_P(PSTR("GET /"), _http_buff);
        _gsm->GPRS_send(_http_buff);
        Serial.println(query_string);
        _gsm->GPRS_send(query_string);     // Send the head of the request
        get_from_flash_P(PSTR(" HTTP/1.1\r\n"), _http_buff);
        _gsm->GPRS_send(_http_buff);
        get_from_flash_P(PSTR("Host: "), _http_buff);
        _gsm->GPRS_send(_http_buff);
        _gsm->GPRS_send(host);    // Send virtual host
        _gsm->GPRS_send("\r\n");
        for(int k=0;k<HTTP_HEADERS_LEN;k++) {
                get_from_flash(&(header_string_table[k]), _http_buff);
                _gsm->GPRS_send(_http_buff);
        }
        _gsm->GPRS_send("\r\n"); // Trailing \r\n to finish the header
        
	PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_end(&child_pt, ret));

	_gsm->GSM_set_callback(HTTP_process_reply);

        PT_WAIT_THREAD(pt, _gsm->PT_recv(&child_pt, ret, "CLOSED", 5000, 0));
        
	_gsm->GSM_set_callback(NULL);

	PT_WAIT_THREAD(pt, PT_backend_end(&child_pt, ret));

	PT_END(pt);
}

int DLHTTP::PT_POST_start(struct pt *pt, char *ret, char *url) {
	static struct pt child_pt;
	PT_BEGIN(pt);
	
	PT_WAIT_THREAD(pt, PT_POST_start(&child_pt, ret, url, 0));

	PT_END(pt);
}

int DLHTTP::PT_POST_start(struct pt *pt, char *ret, char *url, int cl) {
	static struct pt child_pt;
        static char *host, *query_string;
	PT_BEGIN(pt);
	_sent = 0;
        parse_url(url, &host, &query_string);

	PT_WAIT_THREAD(pt, PT_backend_start(&child_pt, ret, host, 80));
        if (*ret != 1) {
		*ret = 0;
		PT_EXIT(pt);
	}
	
	get_from_flash_P(PSTR("POST /"), _http_buff);
        _gsm->GPRS_send(_http_buff);
        _gsm->GPRS_send(query_string);
        get_from_flash_P(PSTR(" HTTP/1.1\r\n"), _http_buff);
        _gsm->GPRS_send(_http_buff);
        get_from_flash_P(PSTR("Host: "), _http_buff);
        _gsm->GPRS_send(_http_buff);
        _gsm->GPRS_send(host);
        _gsm->GPRS_send("\r\n");
        get_from_flash_P(PSTR("Content-Length: "), _http_buff);
        _gsm->GPRS_send(_http_buff);
        _gsm->GPRS_send(cl);
        _gsm->GPRS_send("\r\n");

        for(int k=0;k<HTTP_HEADERS_LEN;k++) {
                get_from_flash(&(header_string_table[k]), _http_buff);
                _gsm->GPRS_send(_http_buff);
        }
        _gsm->GPRS_send("\r\n");
        
        PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_end(&child_pt, ret));
	if (*ret != 1)
		PT_RESTART(pt);

	*ret = 1;
	PT_END(pt);
}

int DLHTTP::PT_POST(struct pt *pt, char *ret, char *data, int len) {
	static struct pt child_pt;
	uint32_t sendsize;
	PT_BEGIN(pt);
	if (!_gsm->CONN_get_flag(CONN_CONNECTED)) {
		*ret = 2;
		PT_EXIT(pt);
	}

        if (data) {
			
                if (_sent == 0) {
                	PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_start(&child_pt, ret)); 
		}
                _sent += len;
		sendsize = _gsm->GPRS_send_get_size();
		if (sendsize == 0) {
			*ret = 3;
			PT_EXIT(pt);
		}

                if (_sent > sendsize) {
			PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_end(&child_pt, ret));
                        _sent = len;
			PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_start(&child_pt, ret));
                }
                Serial.print("Sent: ");
                Serial.println(_sent, DEC);
                _gsm->GPRS_send_raw(data, len);
		*ret = 1;
        } else {
		*ret = 0;
	}
	PT_END(pt);
}

int DLHTTP::PT_POST_end(struct pt *pt, char *ret) {
	static struct pt child_pt;
	PT_BEGIN(pt);

        PT_WAIT_THREAD(pt, _gsm->PT_GPRS_send_end(&child_pt, ret));

	_gsm->GSM_set_callback(HTTP_process_reply);

        PT_WAIT_THREAD(pt, _gsm->PT_recv(&child_pt, ret, "CLOSED", 5000, 0));

	_gsm->GSM_set_callback(NULL);
	
	PT_WAIT_THREAD(pt, PT_backend_end(&child_pt, ret));

	PT_WAIT_THREAD(pt, _gsm->PT_GPRS_check_conn_state(&child_pt, ret));
	
	PT_END(pt);
}

#endif

uint8_t DLHTTP::backend_start(char *host, uint16_t port) {
	int s = 0;
	uint8_t j = 0;
	_gsm->pwr_on();
	backend_err = 999;
	for(j=0;j<10;j++) {
		//s = _gsm->wake_modem();
		if (s) {
			s = _gsm->GPRS_connect(host, port, true);
			if (s == GPRSS_CONNECT_OK) {
				_gsm->GPRS_send_start();
				return 1;	
			}
		}
		delay(10);
	}
	return 0;
}


uint8_t DLHTTP::backend_end() {
	uint8_t s = 0;
	s = _gsm->GPRS_close();
	return s;
}

uint8_t DLHTTP::get_err_code() {
	return (*_backend_err);
}

void DLHTTP::parse_url(char *url, char **host, char **query_string) {
	get_from_flash_P(PSTR("http://"), _http_buff);
	char *httpb = strstr(url, _http_buff);
	char *cptr = httpb;
	if (httpb) // Lets skip the http://
		cptr = cptr + 7;
	*host = cptr; // Then comes the host portion
	*query_string = strstr(cptr, "/") + 1; // Then the query string
	*(*query_string - 1) = 0; // Lets  terminate the string and overwrite the first slash
}

uint8_t DLHTTP::GET(char *url) {
	char *host, *query_string;

	parse_url(url, &host, &query_string);
	if (!backend_start(host, 80))
		return 0;
	
	get_from_flash_P(PSTR("GET /"), _http_buff);
	_gsm->GPRS_send(_http_buff);
	Serial.println(query_string);
	_gsm->GPRS_send(query_string);     // Send the head of the request
	get_from_flash_P(PSTR(" HTTP/1.1\r\n"), _http_buff);
	_gsm->GPRS_send(_http_buff);
	get_from_flash_P(PSTR("Host: "), _http_buff);
	_gsm->GPRS_send(_http_buff);
	_gsm->GPRS_send(host);    // Send virtual host
	_gsm->GPRS_send("\r\n");
	for(int k=0;k<HTTP_HEADERS_LEN;k++) {
		get_from_flash(&(header_string_table[k]), _http_buff);	
		_gsm->GPRS_send(_http_buff);	
	}
	_gsm->GPRS_send("\r\n"); // Trailing \r\n to finish the header
	_gsm->GPRS_send_end();

	_gsm->GSM_fast_read("CLOSED", HTTP_process_reply);

	backend_end();
	return 1;
}

uint8_t DLHTTP::POST_start(char *url) {
	return POST_start(url, 0);
}

uint8_t DLHTTP::POST_start(char *url, int cl) {
        char *host, *query_string;
        parse_url(url, &host, &query_string);
        if (!backend_start(host, 80))
                return 0;
	get_from_flash_P(PSTR("POST /"), _http_buff);
        _gsm->GPRS_send(_http_buff);
        _gsm->GPRS_send(query_string);
	get_from_flash_P(PSTR(" HTTP/1.1\r\n"), _http_buff);
        _gsm->GPRS_send(_http_buff);
	get_from_flash_P(PSTR("Host: "), _http_buff);
        _gsm->GPRS_send(_http_buff);
        _gsm->GPRS_send(host);
        _gsm->GPRS_send("\r\n");
	get_from_flash_P(PSTR("Content-Length: "), _http_buff);
        _gsm->GPRS_send(_http_buff);
	_gsm->GPRS_send(cl);
	_gsm->GPRS_send("\r\n");

        for(int k=0;k<HTTP_HEADERS_LEN;k++) {
                get_from_flash(&(header_string_table[k]), _http_buff);
                _gsm->GPRS_send(_http_buff);
        }
        _gsm->GPRS_send("\r\n");
	_gsm->GPRS_send_end();
	_sent = 0;
	return 1;
}

void DLHTTP::POST(char *data, int len) {
	if (data) {
		if (_sent == 0)
			_gsm->GPRS_send_start();
		_sent += len;
		if (_sent > _gsm->GPRS_send_get_size()) {
			Serial.println("Data exceed.");
			_gsm->GPRS_send_end();
			_sent = len;
			_gsm->GPRS_send_start();
		}
		Serial.print("Sent: ");
		Serial.println(_sent, DEC);
		_gsm->GPRS_send_raw(data, len);
			
	}
}

uint8_t DLHTTP::POST_end() {
	_gsm->GPRS_send_end();

	_gsm->GSM_fast_read("CLOSED", HTTP_process_reply);	

	backend_end();
	return 1;
}
