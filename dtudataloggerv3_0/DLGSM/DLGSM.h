#ifndef DLGSM_h
#define DLGSM_h

#define HARDWARE_SERIAL 1

#include <Arduino.h>
#include <DLCommon.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#ifndef HARDWARE_SERIAL
#include <SoftwareSerial.h>
#endif
#include <string.h>

#define WATCHDOG 1

#define GSM_PWR 3

//#define GSM_BAUD 9600
#define GSM_BAUD 57600
#define GSM_RX 7
#define GSM_TX 8
#undef GSM_SW_FLOW

#define GPRS_CONN_TIMEOUT 10  // Connection timeout for gprs

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

#define CONN_CONNECTED 0x1
#define CONN_SENDING 0x2
#define CONN_NETWORK 0x4
#define CONN_GPRS_NET 0x8
#define CONN_PWR 0x10


#define GSM_EVENT_STATUS_REQ 1
#define GSM_EVENT_LIVE 2


typedef struct {
	uint8_t ip[4];
	uint16_t port;
	char flags;
} Connection;

class DLGSM
{
	public:
		DLGSM(void);
		void pwr_off(uint8_t force);
		void pwr_off();
		void pwr_on();
#ifdef USE_PT
		int PT_recvline(struct pt *pt, char *ret, char *ptr, int len, int tout, char process);
		int PT_recv(struct pt *pt, char *ret, char *conf, int tout);
		int PT_send_recv(struct pt *pt, char *ret, char *cmd, int tout);
		int PT_send_recv_confirm(struct pt *pt, char *ret, char *cmd, char *conf, int tout);
		int PT_GSM_init(struct pt *pt, char *ret);
		int PT_GPRS_init(struct pt *pt, char *ret);
		int PT_GPRS_check_conn_state(struct pt *pt, char *ret);
		int PT_GSM_event_handler(struct pt *pt, char *ret);
		int PT_check_flag(struct pt *pt, char flag);
		int PT_SMS_send(struct pt *pt, char *ret, char *nr, char *text, int len);
		int PT_SMS_send_end(struct pt *pt);
		int PT_GPRS_connect(struct pt *pt, char *ret, char *server, short port, bool proto);
		int PT_GPRS_send_start(struct pt *pt, char *ret);
		int PT_GPRS_send_end(struct pt *pt, char *ret);
		int PT_GPRS_close(struct pt *pt, char *ret);
		uint8_t wake_modem(struct pt *pt);
		int PT_pwr_on(struct pt *pt);
		int PT_pwr_off(struct pt *pt, uint8_t force);
		int PT_restart(struct pt *pt, char *ret);
		int PT_handle_errors(struct pt *pt);
#else
		uint8_t wake_modem();
#endif
		void init(char *buff, int buffsize, uint8_t tout);
		void debug(uint8_t v);
		uint8_t GSM_init();
		uint8_t GSM_process_line(char *check);	
		uint8_t GSM_process(char *check);
		uint8_t GSM_process(char *check, uint8_t tout);
		void GSM_send(char b);
		void GSM_send(int v);
		void GSM_send(float v);
		void GSM_send(unsigned long v);
		void GSM_send(char *msg);
		void GSM_send(char *msg, int len);
		void GSM_set_timeout(int tout);
		void GSM_Xon();
		void GSM_Xoff();
		int GSM_recvline(char *ptr, int len);
		uint8_t GSM_fast_read(char *until, FUN_callback fun);
		int GSM_recvline_fast(char *ptr, int len);
		void GSM_request_net_status();
		void GSM_get_local_time();
		uint8_t SMS_send(char *nr, char *text, int len);
		uint8_t SMS_send_end();
		void GSM_set_callback(FUN_callback fun);
		uint8_t GPRS_init();
		uint8_t GPRS_connect(char *server, short port, bool proto);
		uint16_t GPRS_send_get_size();
		uint8_t GPRS_send_start();
		void GPRS_send(char *data);
		void GPRS_send_raw(char *data, int len);
		void GPRS_send(float n);
		void GPRS_send(unsigned long n);
		void GPRS_send(int n);
		uint8_t GPRS_send_end();
		uint8_t GPRS_close();
		int8_t GPRS_check_conn_state();
		uint8_t CONN_get_flag(uint8_t f);
		void CONN_set_flag(uint8_t f, uint8_t v);
		char* GSM_get_lac();
		char* GSM_get_ci();
		int8_t GSM_event_handler();
		int8_t available();
	private:
		FUN_callback _gsm_callback;
		bool _gsminit;
		char *_gsm_buff;
		int _gsm_buffsize;
		uint8_t _gsm_tout;
		uint8_t _gsm_ret;
		char _gsm_lac[5];
		char _gsm_ci[5];
		char _gsm_wline;
		uint16_t _sendsize;
		Connection _c;
		uint8_t _DEBUG;
		uint32_t _tout_cnt;
		uint32_t _error_cnt;

};

#endif

