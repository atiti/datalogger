#include "WProgram.h"
#include "DLHTTP.h"

#define Pchar prog_char PROGMEM

#define HTTP_HEADERS_LEN 2
Pchar header_string_0[] = "Connection: close\r\n";
Pchar header_string_1[] = "Content-Type: application/x-www-form-urlencoded\r\n";
PROGMEM const char *header_string_table[] = { header_string_0, header_string_1 };

DLHTTP::DLHTTP()
{

}

void DLHTTP::init(char *http_buff, DLGSM *ptr) {
	_gsm = ptr;
	_http_buff = http_buff;
	_sent = 0;
}

uint8_t DLHTTP::backend_start(char *host, uint16_t port) {
	int s = 0;

	s = _gsm->GPRS_check_conn_state();
	if (s >= GPRSS_IP_STATUS && s < GPRSS_PDP_DEACT) {
		s = _gsm->GPRS_connect(host, port, true);
		if (s == GPRSS_CONNECT_OK || s == 1) {
			_gsm->GPRS_send_start();
			return 1;	
		}
	}
	return 0;
}


uint8_t DLHTTP::backend_end() {
	uint8_t s = 0;
	s = _gsm->GPRS_close();
	return s;
}

void DLHTTP::parse_url(char *url, char **host, char **query_string) {
	char *httpb = strstr(url, "http://");
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
	
	_gsm->GPRS_send("GET /");
	_gsm->GPRS_send(query_string);     // Send the head of the request
	_gsm->GPRS_send(" HTTP/1.1\r\n");
	_gsm->GPRS_send("Host: ");
	_gsm->GPRS_send(host);    // Send virtual host
	_gsm->GPRS_send("\r\n");
	for(int k=0;k<HTTP_HEADERS_LEN;k++) {
		get_from_flash(&(header_string_table[k]), _http_buff);	
		_gsm->GPRS_send(_http_buff);	
	}
	_gsm->GPRS_send("\r\n"); // Trailing \r\n to finish the header
	_gsm->GPRS_send_end();

	_gsm->GSM_fast_read("CLOSED");

	backend_end();
	return 0;
}

uint8_t DLHTTP::POST_start(char *url) {
	return POST_start(url, 0);
}

uint8_t DLHTTP::POST_start(char *url, int cl) {
        char *host, *query_string;
        parse_url(url, &host, &query_string);
        if (!backend_start(host, 80))
                return 0;
        _gsm->GPRS_send("POST /");
        _gsm->GPRS_send(query_string);
        _gsm->GPRS_send(" HTTP/1.1\r\n");
        _gsm->GPRS_send("Host: ");
        _gsm->GPRS_send(host);
        _gsm->GPRS_send("\r\n");
        _gsm->GPRS_send("Content-Length: ");
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

void DLHTTP::POST(char *data) {
	if (data) {
		if (_sent == 0)
			_gsm->GPRS_send_start();	
		_gsm->GPRS_send(data);
		_sent += strlen(data);
		if (_sent >= 1000) {
			_gsm->GPRS_send_end();
			_sent = 0;
		}
	}
}

uint8_t DLHTTP::POST_end() {
	_gsm->GPRS_send_end();

	//_gsm->GSM_process(NULL, 50);
	Serial.println("Read fast now!");
	_gsm->GSM_fast_read("CLOSED");	
	Serial.println("Read fast end!");

	backend_end();
	return 0;
}

