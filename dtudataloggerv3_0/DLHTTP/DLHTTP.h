#ifndef DLHTTP_h
#define DLHTTP_h

#include <Arduino.h>
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <DLGSM.h>

class DLHTTP
{
	public:
		DLHTTP();
		void debug(int v);
		void init(char *http_buff, DLGSM *gsmptr);
		uint8_t backend_start(char *host, uint16_t port);
		uint8_t backend_end();
		uint8_t get_err_code();
		void parse_url(char *url, char **host, char **query_string);
		uint8_t GET(char *url);
		uint8_t POST_start(char *url);
		uint8_t POST_start(char *url, int cl);
		void POST(char *data, int len);
		uint8_t POST_end();
		void process_reply();
	private:
		DLGSM *_gsm;
		char *_http_buff;
		short _sent;
		uint8_t _DEBUG;
		uint8_t *_backend_err;
};

#endif

