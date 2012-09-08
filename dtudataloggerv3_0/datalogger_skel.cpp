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
#include <DS1307RTC.h>

#define RTC PCF_RTC

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
#define _cons_baud 19200

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
enum gsm_states { gsm_init_poff, gsm_idle, gsm_booted, gsm_send_http_status, gsm_upload_data, gsm_firmware_dl, gsm_sms_sysinfo, gsm_sms_get_all_readings, gsm_sms_get_reading, gsm_sms_reboot, gsm_sms_uptime };
static enum gsm_states gsm_curr_state = gsm_init_poff;
static enum gsm_states requested_state = gsm_idle;

DLHTTP http;
DLFileUpload fup;

DLSD sd(SPI_HALF_SPEED,4);

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
uint32_t measure_cnt = 0;

void reboot() {
	int i;
	for(i=0;i<NUM_FILES;i++) {
		sd.close(i);
	}
	cfg.wdt_event();	
	reboot_now();
}

void sys_log_message(char *msg) {
	static int sd_error = 0;
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
                        	Serial.print("SD write error fs: ");   
				Serial.println(filesize, DEC);     
        			sd.error();
	        		sd_error++;
			} else {
				sd_error = 0;
			}
			if (sd_error == 10) // Wow the SD card kinda sucks
				reboot();
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
	int cdown = 0;
	bool led = false;
	set_bandgap(1104, 0);
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
	pinMode(STATUS_LED_PIN, OUTPUT); // Status LED
	pinMode(WATCHDOG_PIN, OUTPUT);
	digitalWrite(STATUS_LED_PIN, HIGH);
	digitalWrite(WATCHDOG_PIN, HIGH);
	_cons_serial.begin(_cons_baud);
	for(ret = 0;ret < 100;ret++) {
		_cons_serial.print("a");
	}

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

	wdt_enable(WDTO_8S); 
	setSyncProvider(RTC.get); // Setup time provider to RTC
	if(timeStatus()!= timeSet) {
		get_from_flash_P(PSTR("RTC fail!"), log_buff);
	} else {
		get_from_flash_P(PSTR("RTC ok!"), log_buff);
	}
	_cons_serial.println(log_buff);
        setTime(RTC.get());
	ext_wdt_reset();
	dl_start_time = now();
	wdt_reset();
	wdt_disable();

  	// Initialize GSM
  	gsm.init(gsm_buff, GSM_BUFF_SIZE, 5);
  	gsm.debug(0);

	// Initialize HTTP
	http.init(gsm_buff, &gsm);
        ext_wdt_reset();

	// Initialize SD
	sd.debug(0);
	cdown = 0;
	while (ret != 1) { // No debugging
        	ret = sd.init();
        	if (ret == 1) {
                	get_from_flash_P(PSTR("SD ok!"), log_buff);
			_cons_serial.println(log_buff);
		} else {
			sd.error();
			sd.reset();
		}
		led = !led;
		digitalWrite(STATUS_LED_PIN, led);
	        ext_wdt_reset();
        	delay(200);
		cdown++;
		if (cdown == 10)
			reboot();

	}
        ext_wdt_reset();

	measure.init(); // Initialize IO with buffers
	//analog.set_int_fun(int_routine); // Set up the interrupt handler for events
	measure.debug(1); // Turn on analog debug
	
	// Config file loading
	cfg.init(&sd, &measure, log_buff, LOG_BUFF_SIZE);

	cfg.load();
	config = cfg.get_config();
	if (config->http_status_time == 0)
		config->http_status_time = 1*60; // 1 min
	if (config->http_upload_time == 0)
		config->http_upload_time = 10*60; // 10 min

	//config->http_status_time = 99999;
	//config->http_upload_time = 99999;
	//config->measure_time = 10;
	//config->sampling_rate = 5;

	config->num_samples = config->sampling_rate * config->measure_time;
	config->sampling_delay = 1000 / config->sampling_rate;

	// HAX
	if (strlen(config->HTTP_URL) <= 1) {
		Serial.println("URL HAX");
		sprintf(config->HTTP_URL, "http://dl2.zsuatt.com/");
	}

        ext_wdt_reset();

	// File upload init, depends on: config, sd, http
	fup.init(config, &sd, &http, tmp_buff, TMP_BUFF_SIZE); 

#ifdef HAS_EXT_SERIAL
	// External serial launch
	_extserial.begin(EXT_SER_BAUD);
#endif
        ext_wdt_reset();

	digitalWrite(8, HIGH);
	digitalWrite(10, HIGH);

	// Finally enable our internal watchdog
	wdt_enable(WDTO_8S); 
        ext_wdt_reset();
	dl_start_time = now();
	_cons_serial.print("Time: ");
	digital_clock_display();
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
	static int sys_cnt = 0;
	bool write_error = false;
	DHT22_ERROR_t errorCode;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, millis() - timestamp >= 10*interval || _cons_serial.available());
		timestamp = millis();
		if (_cons_serial.available()) {
			t = _cons_serial.read();
			if (t == 'u') {
				_cons_serial.println("HTTP upload requested");
				requested_state = gsm_upload_data;
			}
			else if (t == 's') {
				_cons_serial.println("HTTP status requested");
				requested_state = gsm_send_http_status;
			}
			else if (t == 'c') {
/*				_cons_serial.println("Resetting EEPROM");
				sd.reset_files_count();
				sd.reset_saved_count();
				cfg.save_files_count(0);
				cfg.save_files_count(1);
				reboot();
*/
			}
			else if (t == 'r') {
				_cons_serial.println("Rebooting");
				reboot();
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

			fmtUnsigned(now(), smallbuff, 11);
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
			strcat(sys_buff, " M: ");
			fmtUnsigned(measure_cnt, smallbuff, 11);
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
	
			if (sys_cnt == 10) {
				sys_log_message(sys_buff);
				sys_cnt = 0;
			} else
				_cons_serial.print(sys_buff);

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
			sys_cnt++;
		}
	}
	PT_END(pt);
}
/* Measurement protothread
   Tasks:
         - Take all the periodic analog measurements + write to SD
	 - (Event handling?)
*/
static int protothread_measure(struct pt *pt, uint16_t interval) {
	static unsigned long timestamp = 0;
	static int delta_ts = 0;
	static time_t last_measure;
	static uint16_t interval_v;
	static int32_t filesize = 0;
	static short val = 0;

	PT_BEGIN(pt);
	interval_v = interval;
	timestamp = millis();
	while (1) {
		PT_WAIT_UNTIL(pt, (millis() - timestamp) >= interval_v);
		// Logic for calculating delta sampling delays
		// This way the sampling rate remains constant
		delta_ts = millis() - timestamp;		
		timestamp = millis();
		if (delta_ts > interval && delta_ts < (2*interval)) {
			interval_v = interval - (delta_ts - interval);
		}
		else
			interval_v = interval;

		//if (interval_v > interval+500 || interval_v < interval-500)
		//	interval_v = interval;
/*		Serial.print(millis());	
		Serial.print(" ");
		Serial.print(millis()-timestamp, DEC);
		Serial.print(" ");
		Serial.println(interval_v, DEC);		
*/

		measure_cnt = measure.read_all(1);
		if ((now() - last_measure) > config->measure_time || measure_cnt >= config->num_samples) {
			last_measure = now();
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
					requested_state = gsm_upload_data;
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
	static int32_t filesize = 0;
	static time_t last_upload = 0, last_status = 0, last_idle = 0, ctime;
	static struct pt comm_inside_pt;
	static int n;
	char e=0, v;
	char ret=0;
	static SMS_t *sms;	
	static Snap_t snap;
	static double up;
	
	PT_BEGIN(pt);
	last_upload = now();
	//last_status = now();
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
			gsm_curr_state = gsm_booted;
		} else if (gsm_curr_state == gsm_idle) {
			LOG("GSM idle");
	
			PT_WAIT_UNTIL(pt, gsm.available() || (now() - last_status) > config->http_status_time || (now() - last_upload) > config->http_upload_time || requested_state != gsm_idle || (now() - last_idle) > 10);

			if (gsm.available()) {
				PT_WAIT_THREAD(pt, gsm.PT_GSM_event_handler(&comm_inside_pt, &ret));
				if (ret == GSM_EVENT_STATUS_REQ) {
					gsm_curr_state = gsm_send_http_status;
				} else if (ret == GSM_EVENT_REBOOT) {
					gsm_curr_state = gsm_sms_reboot;
				} else if (ret == GSM_EVENT_GET_ALL_READINGS) {
					gsm_curr_state = gsm_sms_get_all_readings;
				} else if (ret == GSM_EVENT_GET_READING) {
					gsm_curr_state = gsm_sms_get_reading;
				} else if (ret == GSM_EVENT_SYSINFO) {
					gsm_curr_state = gsm_sms_sysinfo;
				} else if (ret == GSM_EVENT_UPTIME) {
					gsm_curr_state = gsm_sms_uptime;
				} 
				sms = gsm.get_SMS();
			} else if ((now() - last_status) > config->http_status_time) {
				gsm_curr_state = gsm_send_http_status;
			} else if ((now() - last_upload) > config->http_upload_time) {
				gsm_curr_state = gsm_upload_data;
			} else if (requested_state != gsm_idle) {
				LOG("Switching to requested state");
				gsm_curr_state = requested_state;
				requested_state = gsm_idle;
			} else if ((now() - last_idle) > 10) {
				last_idle = now();
				LOG("Checking for SMS");
				PT_WAIT_THREAD(pt, gsm.PT_SMS_check(&comm_inside_pt, &ret));
                                if (ret == GSM_EVENT_STATUS_REQ) {
                                        gsm_curr_state = gsm_send_http_status;
                                } else if (ret == GSM_EVENT_REBOOT) {
                                        gsm_curr_state = gsm_sms_reboot;
                                } else if (ret == GSM_EVENT_GET_ALL_READINGS) {
                                        gsm_curr_state = gsm_sms_get_all_readings;
                                } else if (ret == GSM_EVENT_GET_READING) {
                                        gsm_curr_state = gsm_sms_get_reading;
                                } else if (ret == GSM_EVENT_SYSINFO) {
                                        gsm_curr_state = gsm_sms_sysinfo;
                                } else if (ret == GSM_EVENT_UPTIME) {
					gsm_curr_state = gsm_sms_uptime;
				}
				sms = gsm.get_SMS();
			}
		} else if (gsm_curr_state == gsm_send_http_status) {
			LOG("HTTP status");
			last_status = now();
			*tmp_buff = '\0';
			strcat(tmp_buff, config->HTTP_URL);
                        strcat_P(tmp_buff, PSTR("status.php"));
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
			filesize = sd.open(DATALOG, O_READ);
			sd.close(DATALOG);
                        strcat_P(tmp_buff, PSTR("&cls="));
			fmtUnsigned(filesize, smallbuff, 12);
			strcat(tmp_buff, smallbuff);
			strcat_P(tmp_buff, PSTR("&v="));
			fmtUnsigned(get_supply_voltage(), smallbuff, 12);
			strcat(tmp_buff, smallbuff);
			
			PT_WAIT_THREAD(pt, http.PT_GET(&comm_child_pt, &ret, tmp_buff));
                        get_from_flash_P(PSTR("R: "), tmp_buff);
                        _cons_serial.print(tmp_buff);
                        _cons_serial.print(v, DEC);
			_cons_serial.print(" HTTP: ");
			_cons_serial.println(http.get_err_code(), DEC);

			// Syncronise RTC to server time
			if (RTC.get() < (now()-360) || RTC.get() > (now()+360)) {
				sys_log_message("Syncing RTC time");
				RTC.set(now());
			}

			if (ret) {
				gsm_curr_state = gsm_idle;
			}
                        //PT_WAIT_THREAD(pt, gsm.PT_pwr_off(&comm_inside_pt, 0));
			//gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_booted) { 
			PT_WAIT_THREAD(pt, gsm.PT_GPRS_check_conn_state(&comm_inside_pt, &ret));

			sprintf(tmp_buff, "Datalogger %d booted.", config->id);
			Serial.println(tmp_buff);
			PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, "+4527148803", tmp_buff, strlen(tmp_buff)));
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_upload_data) {
			LOG("HTTP upload");
			n = sd.get_saved_count(DATALOG);
			//sd.set_files_count(DATALOG, sd.get_files_count(DATALOG)+1);
			//cfg.save_files_count(0);
			if (n <= (sd.get_files_count(DATALOG)-1)) {			
				PT_WAIT_THREAD(pt, fup.PT_upload(&comm_inside_pt, &ret, DATALOG_READONLY, n));
				if (ret == 1) {
					LOG("Upload successful");
					sd.set_saved_count(DATALOG, n+1);
					cfg.save_files_count(1);
				} else if (ret == 2) {
					LOG("File doesnt exist");
					sd.set_saved_count(DATALOG, n+1);
					cfg.save_files_count(1);
				} else {
					sprintf(tmp_buff, "Upload failed: %d", ret);
					LOG(tmp_buff);
				}
			}
			last_upload = now();
                        //PT_WAIT_THREAD(pt, gsm.PT_pwr_off(&comm_inside_pt, 0));
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_sms_sysinfo) {	
			strcpy(tmp_buff, sys_buff);
                        PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, sms->number, tmp_buff, strlen(tmp_buff)));
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_sms_get_all_readings) {
			*(tmp_buff) = '\0';
			for(v=0;v<NUM_IO;v++) {
				if (measure.snapshot(&snap, v)) {
					sprintf(smallbuff, "p%d ", v);
					strcat(tmp_buff, smallbuff);
					fmtDouble((double)snap.val, 1, smallbuff, 12);
					strcat(tmp_buff, smallbuff);
					if (snap.std_dev > 0.0) {
						strcat(tmp_buff, " ");
						fmtDouble((double)snap.std_dev, 1, smallbuff, 12);
						strcat(tmp_buff, smallbuff);
					}
				}
				strcat(tmp_buff, "\n");
			}
			Serial.println(tmp_buff);
			PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, sms->number, tmp_buff, strlen(tmp_buff)));
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_sms_get_reading) {
			*(tmp_buff) = '\0';
			v = 0+atoi(sms->message+2);
			if (v >= 0 && v < NUM_IO) {
				if (measure.snapshot(&snap, v)) {
					sprintf(smallbuff, "Port: %d\n", v);
                                        strcat(tmp_buff, smallbuff);
					sprintf(smallbuff, "Value: ");
					strcat(tmp_buff, smallbuff);
                                        fmtDouble((double)snap.val, 1, smallbuff, 12);
                                        strcat(tmp_buff, smallbuff);
					sprintf(smallbuff, "\nStd.Dev: ");
					strcat(tmp_buff, smallbuff);
					fmtDouble((double)snap.std_dev, 2, smallbuff, 12);
					strcat(tmp_buff, smallbuff);
					sprintf(smallbuff, "\nMin: ");
					strcat(tmp_buff, smallbuff);
					fmtDouble((double)snap.min, 2, smallbuff, 12);
					strcat(tmp_buff, smallbuff);
					sprintf(smallbuff, "\nMax: ");
					strcat(tmp_buff, smallbuff);
					fmtDouble((double)snap.max, 2, smallbuff, 12);
					strcat(tmp_buff, smallbuff);
					sprintf(smallbuff, "\nVref: ");
					fmtUnsigned(get_supply_voltage(), smallbuff, 12);
					strcat(tmp_buff, smallbuff);
				} else {
					sprintf(tmp_buff, "Invalid port: %d", v);
				}
			} else {
				sprintf(tmp_buff, "Invalid port: %d", v);
			}

			PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, sms->number, tmp_buff, strlen(tmp_buff)));
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_sms_reboot) {
			strcpy(tmp_buff, "Rebooting!");
                        PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, sms->number, tmp_buff, strlen(tmp_buff)));
			reboot();
			gsm_curr_state = gsm_idle;
		} else if (gsm_curr_state == gsm_sms_uptime) {
			ctime = now() - dl_start_time;
			up = ctime / 3600.0;
			strcpy(tmp_buff, "Uptime (h): ");
			fmtDouble(up, 3, smallbuff, 12);
			strcat(tmp_buff, smallbuff);
			strcat(tmp_buff, "\nWDT: ");
			fmtUnsigned(cfg.get_wdt_events(), smallbuff, 12);
			strcat(tmp_buff, smallbuff);
			strcat(tmp_buff, "\nEPR: ");
			fmtUnsigned(cfg.get_eeprom_events(), smallbuff, 12);
			strcat(tmp_buff, smallbuff);
			Serial.println(tmp_buff);
			PT_WAIT_THREAD(pt, gsm.PT_SMS_send(&comm_inside_pt, &ret, sms->number, tmp_buff, strlen(tmp_buff)));
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
	static long timestamp, lastevent;
	static long filesize;
	static bool write_error = false;
	PT_BEGIN(pt);
	while (1) {
		PT_WAIT_UNTIL(pt, measure.check_event() == 1 || (millis() - timestamp) > 1000);
		timestamp = millis();
		if ((millis() - lastevent) < 500) {
			measure.reset_event();
		}
		else if (measure.check_event()) {
			lastevent = millis();
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
				if (filesize != -1) {
					write_error = sd.write(DATALOG, log_buff);
					if (write_error) {
						LOG("Failed to write event");
					}
				}
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
		wdt_reset();
		timestamp = millis();
	        digitalWrite(WATCHDOG_PIN, LOW);
		PT_WAIT_UNTIL(pt, (millis() - timestamp) >= 40);
		digitalWrite(WATCHDOG_PIN, HIGH);

		//ext_wdt_reset();
	}
	PT_END(pt);
}

#define SET_IF_MAX(v,a) { \
	if (a > v) v = a; \
	} 

void loop() {
	uint32_t ts = millis();
	protothread_sys(&threads[THREAD_SYS].pt, 1000);
	SET_IF_MAX(threads[THREAD_SYS].timing, millis()-ts);
	ts = millis();
	protothread_measure(&threads[THREAD_MEAS].pt, config->sampling_delay);
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

