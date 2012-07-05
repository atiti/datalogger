#define USE_PT

#include <Arduino.h>
#ifdef USE_PT
#include <pt.h>
#endif
#include <HardwareSerial.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Time.h>
#include <PCF8583.h>
#include <SoftwareSerial.h>
#include <Fat16.h>
#include <DLCommon.h>
#include <DLConfig.h>
#include <DLMeasure.h>
#include <DLSD.h>
#include <DLGSM.h>
#include <DLHTTP.h>

#define MAX_FILESIZE 50000

#define _cons_serial Serial
#define _cons_baud 57600

#define LOG(v) {  \
	_cons_serial.print(millis()); \
	_cons_serial.print(": "); \
	_cons_serial.println(v); \
	}

DLConfig cfg;
Config *config = NULL;

#define TMP_BUFF_SIZE 200
char tmp_buff[TMP_BUFF_SIZE]; 


#define LOG_BUFF_SIZE 2048
char log_buff[LOG_BUFF_SIZE];
char smallbuff[20];

#define GSM_BUFF_SIZE 200
DLGSM gsm;
char gsm_buff[GSM_BUFF_SIZE];
enum gsm_states { gsm_init_poff, gsm_idle, gsm_send_sms_status, gsm_send_http_status, gsm_upload_data, gsm_firmware_dl };
static enum gsm_states gsm_curr_state = gsm_init_poff;

DLHTTP http;

DLSD sd(0,4);

// IO setup
DLMeasure measure;

typedef struct Thread_t {
	struct pt pt;
	int timing;
};

#define NUM_THREADS 3
#define THREAD_SYS 0
#define THREAD_MEAS 1
#define THREAD_COMM 2
Thread_t threads[NUM_THREADS];
static struct pt pt_sys, pt_measure, pt_comm, comm_child_pt; 

/* Static variables for threads */
/* System thread */
static char led_touts[5] = {10, 50, 100, 150, 200};

/* Communication thread */
static int u = 0;



void setup() {
	int ret = 0;

	wdt_disable();
	delay(200);
	pinMode(0, OUTPUT); // Status LED
	digitalWrite(0, LOW);
	_cons_serial.begin(_cons_baud);
	_cons_serial.println("Setup");


	for(ret=0;ret<NUM_THREADS;ret++) {
		threads[ret].timing=0;
		PT_INIT(&threads[ret].pt);
	}

	get_from_flash_P(PSTR("Mem: "), log_buff);
	_cons_serial.print(log_buff);
	_cons_serial.println(memory_test());

	setSyncProvider(RTC.get); // Setup time provider to RTC
	if(timeStatus()!= timeSet) {
		get_from_flash_P(PSTR("RTC fail!"), log_buff);
	} else {
		get_from_flash_P(PSTR("RTC ok!"), log_buff);
	}
	_cons_serial.println(log_buff);

  	// Initialize GSM
  	gsm.init(gsm_buff, GSM_BUFF_SIZE, 5);
  	gsm.debug(1);

	// Initialize HTTP
	http.init(gsm_buff, &gsm);

	// Initialize SD
	sd.debug(1);
	while (ret != 1) { // No debugging
        	ret = sd.init();
        	if (ret == 1)
                	get_from_flash_P(PSTR("SD ok!"), log_buff);
        	else
                	get_from_flash_P(PSTR("SD fail!"), log_buff);
       		_cons_serial.println(log_buff);
        	_cons_serial.println(ret,DEC);
        	delay(1000);
	}

	measure.init(); // Initialize IO with buffers
	//analog.set_int_fun(int_routine); // Set up the interrupt handler for events
	measure.debug(1); // Turn on analog debug
	
	// Config file loading
	cfg.init(&sd, &measure, log_buff, LOG_BUFF_SIZE);

	cfg.load();
	cfg.load_files_count(0);
	cfg.load_files_count(1);
	config = cfg.get_config();
	if (config->http_status_time == 0)
		config->http_status_time = 10000;
	if (config->http_upload_time == 0)
		config->http_upload_time = 60000;



	PT_INIT(&pt_sys);
	PT_INIT(&pt_measure);
	PT_INIT(&pt_comm);
}

/* System thread
  Tasks: 
	- Pet the internal and external watchdogs 
	- Load config file
	- Status LED
*/
static int protothread_sys(struct pt *pt, int interval) {
	static unsigned long timestamp = 0;
	char t = 0;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, millis() - timestamp >= 10*interval || _cons_serial.available());
		timestamp = millis();
		if (_cons_serial.available()) {
			t = _cons_serial.read();
			if (t == 's') {
				_cons_serial.println("Sending an SMS");
				gsm_curr_state = gsm_send_http_status;

			}
		} else {
			digitalWrite(0, HIGH);
			PT_WAIT_UNTIL(pt, millis() - timestamp >= 200);	
			digitalWrite(0, LOW);
	
			t = gsm.CONN_get_flag(CONN_NETWORK);
			_cons_serial.print(millis());
			_cons_serial.print(": Sys ");
			_cons_serial.print(threads[THREAD_SYS].timing, DEC);
			_cons_serial.print("ms Meas ");
			_cons_serial.print(threads[THREAD_MEAS].timing, DEC);
			_cons_serial.print("ms Comm ");
			_cons_serial.print(threads[THREAD_COMM].timing, DEC);
			_cons_serial.print("ms Net: "); 
			_cons_serial.println(t, DEC);
			threads[THREAD_SYS].timing = 0;
			threads[THREAD_MEAS].timing = 0;
			threads[THREAD_COMM].timing = 0;
		}
	}
	PT_END(pt);
}
/* Measurement protothread
   Tasks:
         - Take all the periodic analog measurements + write to SD
	 - (Event handling?)
*/
static int protothread_measure(struct pt *pt, int interval) {
	static unsigned long timestamp = 0;
	uint32_t i = 0, filesize = 0;
	short val = 0;

	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, millis() - timestamp >= interval);
		timestamp = millis();
		
		i = measure.read_all(1);
		if (i >= 50) {
			measure.get_all();
			measure.time_log_line(log_buff);   
			measure.reset();
			if (sd.is_available() < 0)
				sd.init();
			filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
			if (filesize > MAX_FILESIZE) {
				sd.close(DATALOG);
				sd.increment_file(DATALOG);
				cfg.save_files_count(0);
				filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
			}
			Serial.print(log_buff);
			sd.write(DATALOG, log_buff);
			sd.close(DATALOG);
		}
		LOG("Measured");
	}
	PT_END(pt);
}

/* COMM protothread
   Tasks:
         - Power manage GSM module
	 - Status reporting
	 - Data upload
	 - (Firmware download)
*/
static int protothread_comm(struct pt *pt, int interval) {
	static unsigned long timestamp = 0;
	static uint32_t last_upload = 0, last_status = 0;
	static struct pt comm_inside_pt;
	char e=0, v;
	char ret=0;
	PT_BEGIN(pt);
	timestamp = millis();
	u = 0;
	while (1) {
		if (gsm_curr_state == gsm_init_poff) {
  		      	// Try to switch off the module 
			while (u < 3) {
                		PT_WAIT_THREAD(pt, gsm.PT_send_recv_confirm(&comm_inside_pt, &ret, "AT\r\n", "OK", 1000));
				//gsm.GSM_send("AT\r\n");
				//e = gsm.GSM_process("OK");
				Serial.print("Ret: ");
				Serial.println(ret, DEC);
				if (ret > 0) {
					gsm.pwr_off(1);
					LOG("GSM powered off.");
					gsm_curr_state = gsm_idle;
					u = 3;
					PT_YIELD(pt);
				} else {
					PT_WAIT_UNTIL(pt, e > 0 || (millis() - timestamp > 1000));
					timestamp = millis();
					u++;
				}
			}
			PT_WAIT_THREAD(pt, gsm.PT_GSM_init(&comm_inside_pt, &ret));
			Serial.print("GSM ret: ");
			Serial.println(ret, DEC);
			PT_WAIT_THREAD(pt, gsm.PT_GPRS_init(&comm_inside_pt, &ret));
			Serial.print("GPRS ret: ");
			Serial.println(ret, DEC);
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_idle) {
			LOG("GSM idle");
	
			PT_WAIT_UNTIL(pt, gsm.available() || (millis() - last_status) > 60000 || (millis() - last_upload) > 120000);

			if (gsm.available()) {
				PT_WAIT_THREAD(pt, gsm.PT_GSM_event_handler(&comm_inside_pt, &ret));
				if (ret == GSM_EVENT_STATUS_REQ)
					gsm_curr_state = gsm_send_http_status;
			} else if ((millis() - last_status) > 60000) {
				gsm_curr_state = gsm_send_http_status;
				last_status = millis();
			} else if ((millis() - last_upload) > 120000) {
				LOG("GSM upload...");
				last_upload = millis();
			}
	
			//LOG("Done event handler");
			//PT_WAIT_THREAD(pt, gsm.PT_GPRS_check_conn_state(&comm_inside_pt, &ret));
			//LOG("Done conn check");
			//PT_YIELD(pt);
		} else if (gsm_curr_state == gsm_send_http_status) {
                        strcpy_P(tmp_buff, PSTR("http://dl2.zsuatt.com/status.php"));
                        strcat_P(tmp_buff, PSTR("?id="));
                        fmtUnsigned(config->id, smallbuff, 10);
                        strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&gl="));
                        strcat(tmp_buff, gsm.GSM_get_lac());
                        strcat_P(tmp_buff, PSTR("&gi="));
                        strcat(tmp_buff, gsm.GSM_get_ci());
                        strcat_P(tmp_buff, PSTR("&t="));
                        //fmtUnsigned(analog_values[15], smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&ts="));
                        fmtUnsigned(now(), smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&u="));
                        fmtUnsigned(millis(), smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
                        u = sd.get_files_count(DATALOG);
                        strcat_P(tmp_buff, PSTR("&cl="));
                        fmtUnsigned(u, smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&cls=0"));
                        //v = http.GET(log_buff);
                        PT_WAIT_THREAD(pt, http.PT_GET(&comm_child_pt, &ret, tmp_buff));
                        get_from_flash_P(PSTR("R: "), tmp_buff);
                        Serial.print(tmp_buff);
                        Serial.println(v, DEC);
			if (ret) {
				gsm_curr_state = gsm_idle;
			}
/*			PT_WAIT_THREAD(pt, gsm.PT_GPRS_check_conn_state(&comm_inside_pt, &ret));
  			if (ret >= GPRSS_IP_STATUS && ret < GPRSS_PDP_DEACT) {
				PT_WAIT_THREAD(pt, gsm.PT_GPRS_connect(&comm_inside_pt, &ret, "46.4.106.217", 80, true));
				if (ret == 1) {
        				Serial.println("Connect OK");
					PT_WAIT_THREAD(pt, gsm.PT_GPRS_send_start(&comm_inside_pt, &ret));
        				gsm.GPRS_send("GET /test.php HTTP/1.0\r\n\r\n");
					PT_WAIT_THREAD(pt, gsm.PT_GPRS_send_end(&comm_inside_pt, &ret));
        				PT_WAIT_THREAD(pt, gsm.PT_recv(&comm_inside_pt, &ret, "CLOSED", 3000));	
        				PT_WAIT_THREAD(pt, gsm.PT_GPRS_close(&comm_inside_pt, &ret));
        				if (ret)
          					Serial.println("Close OK");
        				else
          					Serial.println("Close failed");
    					gsm_curr_state = gsm_idle;
				} else {
        				Serial.print("Connect failed 2: ");
					Serial.println(ret, DEC);
				}
			} else if (ret < GPRSS_IP_STATUS) {
				PT_WAIT_THREAD(pt, gsm.PT_GPRS_init(&comm_inside_pt, &ret));
				Serial.print("GPRS init done: ");
				Serial.println(ret, DEC);
  			} else {
    				Serial.print("Bad GPRS status: ");
				Serial.println(ret, DEC);
			}
*/
			//gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_send_sms_status) { 
			LOG("Sent virtual status");
			PT_WAIT_THREAD(pt, gsm.PT_GPRS_check_conn_state(&comm_inside_pt, &ret));

			sprintf(tmp_buff, "Hello world");
			PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, "+4527148803", tmp_buff, strlen(tmp_buff)));
			gsm_curr_state = gsm_idle;
			PT_YIELD(pt);
		} else {	
			PT_WAIT_UNTIL(pt, millis() - timestamp >= (10*interval));
			timestamp = millis();
			LOG("Empty GSM tick");
		}
	}
	PT_END(pt);	
}

#define SET_IF_MAX(v,a) { \
	if (a > v) v = a; \
	} 

void loop() {
	int ts = millis();
	protothread_sys(&threads[THREAD_SYS].pt, 100);
	SET_IF_MAX(threads[THREAD_SYS].timing, millis()-ts);
	ts = millis();
	protothread_measure(&threads[THREAD_MEAS].pt, 200);
	SET_IF_MAX(threads[THREAD_MEAS].timing, millis()-ts);
	ts = millis();
	protothread_comm(&threads[THREAD_COMM].pt, 500);	
	SET_IF_MAX(threads[THREAD_COMM].timing, millis()-ts);	
}

