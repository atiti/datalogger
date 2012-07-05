#ifndef DLHTTP_h
#define DLHTTP_h

#include <Arduino.h>
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <DLGSM.h>

static struct pt http_child_pt;

class DLHTTP
{
	public:
		DLHTTP();
		void debug(int v);
		void init(char *http_buff, DLGSM *gsmptr);
#ifdef USE_PT
		int PT_backend_start(struct pt *pt, char *ret, char *host, uint16_t port);
		int PT_backend_end(struct pt *pt, char *ret);
		int PT_GET(struct pt *pt, char *ret, char *url);
		int PT_POST_start(struct pt *pt, char *ret, char *url);
		int PT_POST_start(struct pt *pt, char *ret, char *url, int cl);
		int PT_POST(struct pt *pt, char *ret, char *data, int len);
		int PT_POST_end(struct pt *pt, char *ret);
#endif		
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

