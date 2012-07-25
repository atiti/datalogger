#define USE_PT
#define ZSUATT_DATALOGGER 1

#include <Arduino.h>
#ifdef USE_PT
#include <pt.h>
#endif
#include <HardwareSerial.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Time.h>
#include <PCF8583.h>
#include <SoftwareSerial.h>
#include <DLCommon.h>
#include <DLConfig.h>
#include <DLMeasure.h>
#include <DLSD.h>
#include <DLGSM.h>
#include <DLHTTP.h>
#include <DLFileUpload.h>
#include <DHT22.h>

#define WATCHDOG_PIN 2
#define DHT22_PIN 14 
#undef HAS_EXT_SERIAL

#undef HAS_DHT22
DHT22 myDHT22(DHT22_PIN);

//#define SHOW_MEASURE_LOGS 1

// Pin location for the status led
#define STATUS_LED_PIN 0

// Show the current voltage if the difference between the average
// and the current is larger than the threshold, otherwise show average
// Average gives a better picture on the long run
#define VOLTAGE_THRESHOLD 10

// Maximum File size for the data logs
#define MAX_FILESIZE 50000
// Maximum file size for the serial logs
#define SERIAL_MAX_FILESIZE 100000000

// Define a serial port as console (can be remapped to different hardware serial
// Or software serial
#define _cons_serial Serial
#define _cons_baud 57600

// External serial port configuration
#define EXT_SER_RX 14 
#define EXT_SER_TX 13
#define EXT_SER_BAUD 19200

// Logging macro, can be disabled on production
#define LOG(v) {  \
	_cons_serial.print(millis()); \
	_cons_serial.print(": "); \
	_cons_serial.println(v); \
	}

/*   ============ The variables below are for editing at your own risk! =========== */

DLConfig cfg;
Config *config = NULL;

#define SYS_BUFF_SIZE 200
char sys_buff[SYS_BUFF_SIZE];

#define TMP_BUFF_SIZE 200
char tmp_buff[TMP_BUFF_SIZE]; 


#define LOG_BUFF_SIZE 512
char log_buff[LOG_BUFF_SIZE];
char smallbuff[20];

#define GSM_BUFF_SIZE 200
DLGSM gsm;
char gsm_buff[GSM_BUFF_SIZE];
enum gsm_states { gsm_init_poff, gsm_idle, gsm_send_sms_status, gsm_send_http_status, gsm_upload_data, gsm_firmware_dl };
static enum gsm_states gsm_curr_state = gsm_init_poff;
static enum gsm_states requested_state = gsm_idle;

DLHTTP http;
DLFileUpload fup;

DLSD sd(0,4);

// IO setup
DLMeasure measure;

#define THREAD_EXCEED_THRESHOLD 50
#define THREAD_RUN_THRESHOLD 600
typedef struct {
	struct pt pt;
	int timing;
	char exceed;
} Thread_t;

#define NUM_THREADS 6
#define THREAD_SYS 0
#define THREAD_MEAS 1
#define THREAD_COMM 2
#define THREAD_SER 3
#define THREAD_EVENT 4
#define THREAD_WDT 5
Thread_t threads[NUM_THREADS];
static struct pt comm_child_pt; 

/* Static variables for threads */
/* System thread */
static time_t dl_start_time = 0;
static char led_touts[5] = {10, 50, 100, 150, 200};

/* Communication thread */
static int u = 0;
#ifdef HAS_EXT_SERIAL
#define EXT_BUFF_SIZE 512 // Matching with the SD card write buffer
SoftwareSerial _extserial(EXT_SER_RX, EXT_SER_TX);
char ext_buff[EXT_BUFF_SIZE];
int ext_buff_pos = 0;
#endif

float curr_temperature = 0.0f;
float curr_humidity = 0.0f;
int curr_voltage = 0;
long total_voltage = 0;
uint32_t main_iter_cnt = 0;

void sys_log_message(char *msg) {
	long filesize;
	bool write_error;
	Serial.print(msg);
	if (sd.is_available() < 0)
		sd.init();
        filesize = sd.open(SYSLOG, O_RDWR | O_CREAT | O_APPEND);
        if (filesize != -1) {
        	if (filesize > MAX_FILESIZE) {
                	sd.close(SYSLOG);
                	sd.increment_file(SYSLOG);
                	cfg.save_files_count(0);
                	filesize = sd.open(SYSLOG, O_RDWR | O_CREAT | O_APPEND);
                }
                if (filesize != -1) {
                	write_error = sd.write(SYSLOG, sys_buff);
                        if (write_error) {
                        	Serial.println("Shit, an SD write error");
                        }
		}
	}
}

void ext_wdt_reset() {
	digitalWrite(WATCHDOG_PIN, LOW);
	delay(10);
	digitalWrite(WATCHDOG_PIN, HIGH);
}
/*
ISR(TIMER1_COMPA_vect) {
	UDR0 = '*';
}
*/

void setup() {
	int ret = 0;
	bool led = false;
	set_bandgap(1080);
	ext_wdt_reset();
	wdt_disable();
/*	noInterrupts();
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	//OCRA = 31250;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12);
	TIMSK1 |= (1 << OCIE1A);
	interrupts();
*/
	delay(200); // Give us some time to start up
	pinMode(STATUS_LED_PIN, OUTPUT); // Status LED
	pinMode(WATCHDOG_PIN, OUTPUT);
	digitalWrite(STATUS_LED_PIN, HIGH);
	digitalWrite(WATCHDOG_PIN, HIGH);
	_cons_serial.begin(_cons_baud);
	_cons_serial.println("Setup");

	for(ret=0;ret<NUM_THREADS;ret++) {
		threads[ret].timing=0;
		threads[ret].exceed=0;
		PT_INIT(&threads[ret].pt);
	}
	ext_wdt_reset();
	get_from_flash_P(PSTR("Mem: "), log_buff);
	_cons_serial.print(log_buff);
	_cons_serial.println(memory_test());
        ext_wdt_reset();

	setSyncProvider(RTC.get); // Setup time provider to RTC
	if(timeStatus()!= timeSet) {
		get_from_flash_P(PSTR("RTC fail!"), log_buff);
	} else {
		get_from_flash_P(PSTR("RTC ok!"), log_buff);
	}
	_cons_serial.println(log_buff);
        ext_wdt_reset();
	dl_start_time = now();


  	// Initialize GSM
  	gsm.init(gsm_buff, GSM_BUFF_SIZE, 5);
  	gsm.debug(1);

	// Initialize HTTP
	http.init(gsm_buff, &gsm);
        ext_wdt_reset();

	// Initialize SD
	sd.debug(0);
	while (ret != 1) { // No debugging
        	ret = sd.init();
        	if (ret == 1)
                	get_from_flash_P(PSTR("SD ok!"), log_buff);
        	else
                	get_from_flash_P(PSTR("SD fail!"), log_buff);
       		_cons_serial.println(log_buff);
        	_cons_serial.println(ret,DEC);
		led = !led;
		digitalWrite(STATUS_LED_PIN, led);
	        ext_wdt_reset();
        	delay(200);
	}
        ext_wdt_reset();

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
		config->http_status_time = 1*60*1000; // 1 min
	if (config->http_upload_time == 0)
		config->http_upload_time = 10*60*1000; // 10 min

        ext_wdt_reset();

	// File upload init, depends on: config, sd, http
	fup.init(config, &sd, &http, tmp_buff, TMP_BUFF_SIZE); 

#ifdef HAS_EXT_SERIAL
	// External serial launch
	_extserial.begin(EXT_SER_BAUD);
#endif
        ext_wdt_reset();

	// Finally enable our internal watchdog
	wdt_enable(WDTO_8S); 
        ext_wdt_reset();
}

/* System thread
  Tasks: 
	- Pet the internal and external watchdogs 
	- Load config file
	- Status LED
	- Track real uptime (millis overflow)
*/
static int protothread_sys(struct pt *pt, int interval) {
	static unsigned long timestamp = 0;
	char t = 0;
	int32_t filesize = 0;
	int tmp_voltage = 0;
	bool write_error = false;
	DHT22_ERROR_t errorCode;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, millis() - timestamp >= 10*interval || _cons_serial.available());
		timestamp = millis();
		wdt_reset();
		if (_cons_serial.available()) {
			t = _cons_serial.read();
			if (t == 'u') {
				_cons_serial.println("HTTP upload requested");
				requested_state = gsm_upload_data;
			}
			if (t == 's') {
				_cons_serial.println("HTTP status requested");
				requested_state = gsm_send_http_status;

			}
		} else {
			digitalWrite(STATUS_LED_PIN, LOW);
			PT_WAIT_UNTIL(pt, millis() - timestamp >= 200);	
			digitalWrite(STATUS_LED_PIN, HIGH);
	
			errorCode = myDHT22.readData();
			if (errorCode == DHT_ERROR_NONE) {
				curr_temperature = myDHT22.getTemperatureC();
				curr_humidity = myDHT22.getHumidity();
			} else if (errorCode != DHT_ERROR_TOOQUICK) {
#ifdef HAS_DHT22
				_cons_serial.print("DHT22 Error: ");
				_cons_serial.println(errorCode, DEC);
#endif
			} 
			
			// Get the supply voltage by using the internal bandgap, do a rolling average of it
			curr_voltage = get_bandgap();
			tmp_voltage = curr_voltage;
			total_voltage -= total_voltage / 16;	
			total_voltage += curr_voltage;
			curr_voltage = total_voltage / 16;

			set_supply_voltage(((tmp_voltage-VOLTAGE_THRESHOLD) > curr_voltage ? tmp_voltage : curr_voltage));
	
			t = gsm.CONN_get_flag(0xff);

			fmtUnsigned(now()-dl_start_time, smallbuff, 11);
			strcpy(sys_buff, smallbuff);
			strcat(sys_buff, ": ");
			fmtUnsigned(main_iter_cnt, smallbuff, 11);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, "Hz Sys ");
			fmtUnsigned(threads[THREAD_SYS].timing, smallbuff, 11);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, "ms Meas ");
			fmtUnsigned(threads[THREAD_MEAS].timing, smallbuff, 11);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, "ms Ev ");
			fmtUnsigned(threads[THREAD_EVENT].timing, smallbuff, 11);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, "ms Comm ");
			fmtUnsigned(threads[THREAD_COMM].timing, smallbuff, 11);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, "ms Net: ");
			fmtUnsigned(t, smallbuff, 11);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, " T: ");
			dtostrf(curr_temperature, 4, 3, smallbuff);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, " H: ");
			dtostrf(curr_humidity, 4, 3, smallbuff);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, " V: ");
			fmtUnsigned(get_supply_voltage(), smallbuff, 10);
			strcat(sys_buff, smallbuff);
			strcat(sys_buff, "\r\n");
	
			sys_log_message(sys_buff);
			
			// Check health of device
			for(t=0;t<NUM_THREADS;t++) {
				if (threads[t].timing > THREAD_RUN_THRESHOLD)
					threads[t].exceed++;
				else
					threads[t].exceed = 0;

				if (threads[t].exceed > THREAD_EXCEED_THRESHOLD) {
					sys_log_message("Thread theshold reached. Rebooting\r\n");
					reboot();		
				}
				threads[t].timing = 0;
			}

			main_iter_cnt = 0;
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
	int32_t i = 0, filesize = 0;
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
			if (filesize != -1) {
				if (filesize > MAX_FILESIZE) {
					sd.close(DATALOG);
					sd.increment_file(DATALOG);
					cfg.save_files_count(0);
					filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
				}
				sd.write(DATALOG, log_buff);
				//sd.close(DATALOG);
			}
			Serial.print(log_buff);
		}
#ifdef SHOW_MEASURE_LOGS
		LOG("Measured");
#endif
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
	static int32_t last_upload = 0, last_status = 0, filesize = 0;
	static struct pt comm_inside_pt;
	char e=0, v;
	char ret=0;
	PT_BEGIN(pt);
	timestamp = millis();
	u = 0;
	while (1) {
		if (gsm_curr_state == gsm_init_poff) {
			timestamp = millis();
//			PT_WAIT_THREAD(pt, gsm.PT_pwr_on(&comm_inside_pt));
			PT_WAIT_THREAD(pt, gsm.PT_restart(&comm_inside_pt, &ret));
			PT_WAIT_UNTIL(pt, (millis() - timestamp) > 5000);
			timestamp = millis();
			//PT_WAIT_WHILE(pt, gsm.CONN_get_flag(CONN_NETWORK) == 0);
			PT_WAIT_THREAD(pt, gsm.PT_check_flag(&comm_inside_pt, CONN_NETWORK));
			PT_WAIT_THREAD(pt, gsm.PT_GSM_init(&comm_inside_pt, &ret));
			Serial.print("GSM ret: ");
			Serial.println(ret, DEC);
			PT_WAIT_UNTIL(pt, (millis() - timestamp) > 3000);
			//PT_WAIT_WHILE(pt, gsm.CONN_get_flag(CONN_GPRS_NET) == 0);
			PT_WAIT_THREAD(pt, gsm.PT_check_flag(&comm_inside_pt, CONN_GPRS_NET));
			PT_WAIT_THREAD(pt, gsm.PT_GPRS_init(&comm_inside_pt, &ret));
			Serial.print("GPRS ret: ");
			Serial.println(ret, DEC); 
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_idle) {
			LOG("GSM idle");
	
			PT_WAIT_UNTIL(pt, gsm.available() || (millis() - last_status) > config->http_status_time || (millis() - last_upload) > config->http_upload_time || requested_state != gsm_idle);

			if (gsm.available()) {
				PT_WAIT_THREAD(pt, gsm.PT_GSM_event_handler(&comm_inside_pt, &ret));
				if (ret == GSM_EVENT_STATUS_REQ)
					gsm_curr_state = gsm_upload_data;
			} else if ((millis() - last_status) > config->http_status_time) {
				gsm_curr_state = gsm_send_http_status;
			} else if ((millis() - last_upload) > config->http_upload_time) {
				LOG("GSM upload...");
				last_upload = millis();
				gsm_curr_state = gsm_upload_data;
			} else if (requested_state != gsm_idle) {
				LOG("Switching to requested state");
				gsm_curr_state = requested_state;
				requested_state = gsm_idle;
			}

		} else if (gsm_curr_state == gsm_send_http_status) {
			last_status = millis();
                        strcpy_P(tmp_buff, PSTR("http://dl2.zsuatt.com/status.php"));
                        strcat_P(tmp_buff, PSTR("?id="));
                        fmtUnsigned(config->id, smallbuff, 10);
                        strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&gl="));
                        strcat(tmp_buff, gsm.GSM_get_lac());
                        strcat_P(tmp_buff, PSTR("&gi="));
                        strcat(tmp_buff, gsm.GSM_get_ci());
                        strcat_P(tmp_buff, PSTR("&t="));
			dtostrf(curr_temperature, 4, 3, smallbuff);
                        strcat(tmp_buff, smallbuff);
			strcat_P(tmp_buff, PSTR("&hum="));
			dtostrf(curr_humidity, 4,3,smallbuff);
			strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&ts="));
                        fmtUnsigned(now(), smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
                        strcat_P(tmp_buff, PSTR("&u="));
                        fmtUnsigned(now()-dl_start_time, smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
                        u = sd.get_files_count(DATALOG);
                        strcat_P(tmp_buff, PSTR("&cl="));
                        fmtUnsigned(u, smallbuff, 12);
                        strcat(tmp_buff, smallbuff);
			filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
			sd.close(DATALOG);
                        strcat_P(tmp_buff, PSTR("&cls="));
			fmtUnsigned(filesize, smallbuff, 12);
			strcat(tmp_buff, smallbuff);
			strcat_P(tmp_buff, PSTR("&v="));
			fmtUnsigned(curr_voltage, smallbuff, 12);
			strcat(tmp_buff, smallbuff);
                        
			PT_WAIT_THREAD(pt, http.PT_GET(&comm_child_pt, &ret, tmp_buff));
                        get_from_flash_P(PSTR("R: "), tmp_buff);
                        Serial.print(tmp_buff);
                        Serial.println(v, DEC);
			if (ret) {
				gsm_curr_state = gsm_idle;
			}
                        //PT_WAIT_THREAD(pt, gsm.PT_pwr_off(&comm_inside_pt, 0));
			//gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_send_sms_status) { 
			LOG("Sent virtual status");
			PT_WAIT_THREAD(pt, gsm.PT_GPRS_check_conn_state(&comm_inside_pt, &ret));

			sprintf(tmp_buff, "Hello world");
			PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, "+4527148803", tmp_buff, strlen(tmp_buff)));
			//PT_WAIT_THREAD(pt, gsm.PT_pwr_off(&comm_inside_pt, 0));
			gsm_curr_state = gsm_idle;
			PT_YIELD(pt);
		} else if (gsm_curr_state == gsm_upload_data) {
			LOG("Doing HTTP upload");
			PT_WAIT_THREAD(pt, fup.PT_upload(&comm_inside_pt, &ret, DATALOG_READONLY, 0));
			if (ret == 1) {
				LOG("Upload successful");
			} else {
				LOG("Upload failed");
			}
                        //PT_WAIT_THREAD(pt, gsm.PT_pwr_off(&comm_inside_pt, 0));
			gsm_curr_state = gsm_idle;
		} else {	
			PT_WAIT_UNTIL(pt, millis() - timestamp >= (10*interval));
			timestamp = millis();
			LOG("Empty GSM tick");
		}
	}
	PT_END(pt);	
}
#ifdef HAS_EXT_SERIAL
static int protothread_serial(struct pt *pt, int interval) {
	static struct pt child_pt;
	static long timestamp;
	int32_t filesize;
	char c;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, (millis() - timestamp >= interval) || _extserial.available());
                timestamp = millis();
		while (_extserial.available() && ext_buff_pos < EXT_BUFF_SIZE) {
			c = _extserial.read();
			_cons_serial.print(c);
			ext_buff[ext_buff_pos] = c;
			ext_buff_pos++;
		}
		if (ext_buff_pos == (EXT_BUFF_SIZE-1)) { // Write to SD
                        if (sd.is_available() < 0)
                                sd.init();
                        filesize = sd.open(SERIALLOG, O_RDWR | O_CREAT | O_APPEND);
                 	if (filesize != -1) {
				if (filesize > SERIAL_MAX_FILESIZE) {
                        	        sd.close(SERIALLOG);
                        	        sd.increment_file(SERIALLOG);
                        	        //cfg.save_files_count(0);
                        	        filesize = sd.open(SERIALLOG, O_RDWR | O_CREAT | O_APPEND);
                       		}
                        	sd.write(SERIALLOG, ext_buff);
	                        //sd.close(SERIALLOG);
				LOG("Wrote external serial data");	
			}
		}
	}
	PT_END(pt);
} 
#endif

static int protothread_event(struct pt *pt, int interval) {
	static struct pt child_pt;
	static long timestamp;
	static long filesize;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, measure.check_event() == 1 || (millis() - timestamp) > 1000);
		timestamp = millis();
		if (measure.check_event()) {
			measure.event_log_line(log_buff);
			measure.reset_event();
		        if (sd.is_available() < 0)
       		         	sd.init();
			filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
			if (filesize != -1) {
				if (filesize > MAX_FILESIZE) {
					sd.close(DATALOG);
					sd.increment_file(DATALOG);
					cfg.save_files_count(0);
					filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
				}
				if (filesize != -1)
					sd.write(DATALOG, log_buff);
			}
			_cons_serial.print(log_buff);
		}
	}
	PT_END(pt);
}

static int protothread_wdt(struct pt *pt, int interval) {
	static struct pt child_pt;
	static long timestamp;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, (millis()-timestamp) >= interval);
		timestamp = millis();
		ext_wdt_reset();
	}
	PT_END(pt);
}

#define SET_IF_MAX(v,a) { \
	if (a > v) v = a; \
	} 

void loop() {
	uint32_t ts = millis();
	protothread_sys(&threads[THREAD_SYS].pt, 100);
	SET_IF_MAX(threads[THREAD_SYS].timing, millis()-ts);
	ts = millis();
	protothread_measure(&threads[THREAD_MEAS].pt, 200);
	SET_IF_MAX(threads[THREAD_MEAS].timing, millis()-ts);
	ts = millis();
	protothread_comm(&threads[THREAD_COMM].pt, 500);	
	SET_IF_MAX(threads[THREAD_COMM].timing, millis()-ts);	
	ts = millis();
#ifdef HAS_EXT_SERIAL
	protothread_serial(&threads[THREAD_SER].pt, 1000);
	SET_IF_MAX(threads[THREAD_SER].timing, millis()-ts);
	ts = millis();
#endif
	protothread_event(&threads[THREAD_EVENT].pt, 1000);
	SET_IF_MAX(threads[THREAD_EVENT].timing, millis()-ts);
	ts = millis();
	protothread_wdt(&threads[THREAD_WDT].pt, 600);
	SET_IF_MAX(threads[THREAD_WDT].timing, millis()-ts);
	main_iter_cnt++;

//	set_sleep_mode(SLEEP_MODE_IDLE);
//	sleep_enable();
//	sleep_mode();
//	sleep_disable(); 
}

