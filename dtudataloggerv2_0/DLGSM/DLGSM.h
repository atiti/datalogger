#ifndef DLGSM_h
#define DLGSM_h

#include "WProgram.h"
#include "WConstants.h"
#include <DLCommon.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <NewSoftSerial.h>
#include <string.h>

#define RX 7
#define TX 8

class DLGSM
{
	public:
		DLGSM();
		void debug(int v);
		void GSM_init();
		void GSM_process();
		void GSM_process_callback();
		void GSM_send(char b);
		void GSM_send(int v);
		void GSM_send(char *msg);
		void GSM_recvline(char *ptr, int len);
		void GSM_request_net_status();
		void GSM_get_local_time();
		void GPRS_init();
		bool GPRS_connect(char *server, int port, bool proto);
		bool GPRS_send_start();
		void GPRS_send(char *data);
		void GPRS_send_raw(char *data, int len);
		void GPRS_send(float n);
		void GPRS_send(unsigned long n);
		void GPRS_send_end();
		void GPRS_close();
		void GPRS_check_conn_state();
	private:
		uint8_t _DEBUG;
};

#endif

