#ifndef DLHTTP_h
#define DLHTTP_h

#include "WProgram.h"
#include "WConstants.h"
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
		void parse_url(char *url, char **host, char **query_string);
		uint8_t GET(char *url);
		uint8_t POST_start(char *url);
		uint8_t POST_start(char *url, int cl);
		void POST(char *data);
		uint8_t POST_end();
	private:
		DLGSM *_gsm;
		char *_http_buff;
		short _sent;
		uint8_t _DEBUG;
};

#endif

