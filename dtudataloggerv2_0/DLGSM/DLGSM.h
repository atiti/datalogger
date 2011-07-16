#ifndef DLGSM_h
#define DLGSM_h

#include "WProgram.h"
#include "WConstants.h"
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <NewSoftSerial.h>
#include <string.h>

#define GSM_BAUD 19200
#define GSM_RX 7
#define GSM_TX 8

#define GPRSS_IP_INITIAL 0
#define GPRSS_IP_START 1
#define GPRSS_IP_CONFIG 2
#define GPRSS_IP_GPRSACT 3
#define GPRSS_IP_STATUS 4
#define GPRSS_TCP_CONNECTING 5
#define GPRSS_UDP_CONNECTING 6
#define GPRSS_CONNECT_OK 7
#define GPRSS_TCP_CLOSING 8
#define GPRSS_UDP_CLOSING 9
#define GPRSS_TCP_CLOSED 10
#define GPRSS_UDP_CLOSED 11
#define GPRSS_PDP_DEACT 12


typedef int (*GSM_callback)(char *, int);

#define CONN_CONNECTED 0x1
#define CONN_SENDING 0x2

typedef struct {
	uint8_t ip[4];
	uint16_t port;
	char flags;
} Connection;

class DLGSM
{
	public:
		DLGSM(void);
		void init(char *buff, int buffsize, int tout);
		void debug(int v);
		int GSM_init();
		int GSM_process(char *check);
		int GSM_process(char *check, int tout);
		void GSM_send(char b);
		void GSM_send(int v);
		void GSM_send(float v);
		void GSM_send(unsigned long v);
		void GSM_send(char *msg);
		void GSM_send(char *msg, int len);
		void GSM_set_timeout(int tout);
		int GSM_recvline(char *ptr, int len);
		void GSM_request_net_status();
		void GSM_get_local_time();
		void GSM_set_callback(GSM_callback fun);
		int GPRS_init();
		int GPRS_connect(char *server, int port, bool proto);
		bool GPRS_send_start();
		void GPRS_send(char *data);
		void GPRS_send_raw(char *data, int len);
		void GPRS_send(float n);
		void GPRS_send(unsigned long n);
		bool GPRS_send_end();
		int GPRS_close();
		int GPRS_check_conn_state();
		char CONN_get_flag(char f);
		void CONN_set_flag(char f, char v);
	private:
		GSM_callback _gsm_callback;
		bool _gsminit;
		char *_gsm_buff;
		int _gsm_buffsize;
		int _gsm_tout;
		int _gsm_ret;
		char _gsm_lac[5];
		char _gsm_ci[5];
		Connection _c;
		uint8_t _DEBUG;
};

#endif

