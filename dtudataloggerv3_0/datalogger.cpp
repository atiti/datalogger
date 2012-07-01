#include <Arduino.h>
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
#include <DLAnalog.h>
#include <DLSD.h>
#include <DLGSM.h>
#include <DLHTTP.h>

#define DEBUG 1
#define WATCHDOG 1

#define LOAD_CONFIG 0
#define IDLE 1
#define MEASURE 2
#define STATUS 3
#define UPLOAD 4
#define EVENT 5

#define MAX_FILESIZE 20000

//#define MEASURE_TIME 5000  //(5*60*1000ms) 
// Config
DLConfig cfg;
Config *config = NULL;

// IO setup
DLAnalog analog(17, 16, 5, 4, 6, 14, LOW);
uint8_t count_values[16] = { 0 };
double analog_values[16] = { 0 };
double std_dev[16] = { 0 };
volatile uint8_t got_event = 0;
long last_measure = millis();
time_t start_measure = now();
time_t last_status = 0; 
time_t last_upload = 0; 
uint8_t initial_start = 3; // Set this to 3 not to send SMS
// SD setup
DLSD sd(0, 4); // Setup FULL_SPEED SPI and CS on pin 10
unsigned long filesize = 0;

// GSM
#define GSM_BUFF_SIZE 60
DLGSM gsm;

// HTTP
DLHTTP http;

#define LOG_BUFF_SIZE 111
time_t lt = now();
uint8_t state = LOAD_CONFIG; // lets start at IDLE state
uint8_t lstate = IDLE;
char log_buff[LOG_BUFF_SIZE];
char gsm_buff[GSM_BUFF_SIZE];
char smallbuff[12];
uint16_t p, u, v;
char ret = 0;

void int_routine() {
  analog.read_all(1);
  got_event = 1;
}

// File Upload function
// FIXME: Should be in it's own library or HTTP/SD library
uint8_t upload_file(uint8_t fd) {
  unsigned long filesize = 0;
  unsigned long sent_counter = 0;
  short partlength = 4000; // Set the POST part length
  uint8_t err = 0, ret = 1;
  int parts = 1, i = 0;
  uint8_t checksum = 0;
  filesize = sd.open(fd, O_READ); // Open file read only
  if (filesize == -1)
    return 1;
  sd.rewind(fd); // Rewind to the beginning
 
  // Split up files into multi parts
  if (filesize > partlength) { 
    parts = filesize / partlength; 
    if ((filesize % partlength) != 0) // If we cant do round splits, then add an extra part for the leftovers
      parts++;  
  }
 
  // Start sending the parts
  i = 0;
  while(i < parts) { // Send parts
    if (Serial.available()) {
      ret = Serial.read();
      if (ret == 's')
       break;     
    }
    // Here the request header is constructed
    strcpy_P(log_buff, PSTR("http://dl2.zsuatt.com/upload.php"));
    strcat_P(log_buff, PSTR("?id="));   // Datalogger ID
    fmtUnsigned(config->id, smallbuff, 10);
    strcat(log_buff, smallbuff);
    //strcat(log_buff, itoa(config->id, smallbuff, 10));
    strcat_P(log_buff, PSTR("&fi=")); // File ID
    fmtUnsigned(fd, smallbuff, 10);
    strcat(log_buff, smallbuff);
    //strcat(log_buff, itoa(fd, smallbuff, 10));
    strcat_P(log_buff, PSTR("&fc=")); // Current file count
    fmtUnsigned(sd.get_files_count(fd), smallbuff, 10);
    strcat(log_buff, smallbuff);
    //strcat(log_buff, ltoa(sd.get_files_count(fd), smallbuff, 10));
    strcat_P(log_buff, PSTR("&p=")); // Current part number
    fmtUnsigned(i, smallbuff, 10);
    strcat(log_buff, smallbuff);
    //strcat(log_buff, itoa(i, smallbuff, 10));
    strcat_P(log_buff, PSTR("&tp=")); // Total number of parts
    fmtUnsigned(parts-1, smallbuff, 10);
    strcat(log_buff, smallbuff);
    //strcat(log_buff, itoa(parts, smallbuff, 10));
    strcat_P(log_buff, PSTR("&fs=")); // Total file size
    fmtUnsigned(filesize, smallbuff, 10);
    strcat(log_buff, smallbuff);
    //strcat(log_buff, ltoa(filesize, smallbuff, 10));
    int cps = partlength; // Assume the biggest part size
    if ((filesize - (i*partlength)) < partlength) // If we are reaching the end of the data, calculate POST length
      cps = filesize - (i*partlength);   
    unsigned long stime = millis();
    sent_counter = 0;
    checksum = 0;
    
    // seek to the correct pos in the open file
    sd.seek(fd, (i*partlength));
    
    // Start the POST request with the URL
    if (http.POST_start(log_buff, cps)) {
      //err = 0; // Reset error counter, we should be good to go again
      while (ret > 0 && sent_counter < cps) { // Send the content of the request
        memset(log_buff, 0, LOG_BUFF_SIZE-1);
        // Read the next chunk
        ret = sd.read(fd, log_buff, LOG_BUFF_SIZE-1);
        // Calculate XOR checksum of the chunk
        //checksum ^= get_checksum(log_buff);
        // POST the chunk
        //Serial.println(ret, DEC);
        http.POST(log_buff, ret); 
        sent_counter += ret;
        wdt_reset();
      }  
      http.POST_end();
      stime = millis() - stime;
#if DEBUG
      Serial.print("PL:");
      Serial.println(stime);
      Serial.print("CS:");
      Serial.println(checksum, DEC);
#endif
      ret = http.get_err_code();
      Serial.print("RET:");
      Serial.println(ret, DEC);
      if (ret == 100) {
        i++; // Successful POST, lets do the next part
        err = 0;  
    } else {
        err++; 
      }
    } else {
#if DEBUG 
      Serial.println("PF!");
#endif
      err++; // Increment error counter
    }
    Serial.print("ERR: ");
    Serial.println(err, DEC);
    if (err > 5) {
      sd.close(fd);
      get_from_flash_P(PSTR("GU."), log_buff); // Giving up
      Serial.println(log_buff);
      return 0;
    }
    wdt_reset();
  }
  sd.close(fd);
  ret = http.get_err_code();
  if (ret == 100)
    return 1;
  else
    return 0;
}

void setup() {
  int8_t e = -1;
  wdt_disable(); // Disable watchdog
  Serial.begin(57600); // Initialize serial
  get_from_flash_P(PSTR("** DL v2.1.0"), log_buff);  // Print welcome
  Serial.println(log_buff);
  
  
/* lot of flash
*/
  get_from_flash_P(PSTR("Mem: "), log_buff);
  Serial.print(log_buff);
  Serial.println(memory_test());


  setSyncProvider(RTC.get); // Setup time provider to RTC
  if(timeStatus()!= timeSet) { 
    get_from_flash_P(PSTR("RTC fail!"), log_buff);
  } else {
    get_from_flash_P(PSTR("RTC ok!"), log_buff);
  }
  Serial.println(log_buff); 
 

  // SD init
  sd.debug(1);
  while (ret != 1) { // No debugging
  	ret = sd.init();
 	if (ret == 1)
    		get_from_flash_P(PSTR("SD ok!"), log_buff);
 	else
    		get_from_flash_P(PSTR("SD fail!"), log_buff);
 	Serial.println(log_buff);
  	Serial.println(ret,DEC);
	delay(1000);
  }
 
 
  // AT this point we proceed, but RTC/SD might be broken...
    
  analog.init(analog_values, std_dev, count_values, 60); // Initialize IO with buffers
  //analog.set_int_fun(int_routine); // Set up the interrupt handler for events
  analog.debug(1); // Turn on analog debug

    // Config file loading
  cfg.init(&sd, &analog, log_buff, LOG_BUFF_SIZE);
  

  // Initialize GSM
  gsm.init(gsm_buff, GSM_BUFF_SIZE, 5);
  gsm.debug(1);

  // Try to switch off the module 
  for(u=0;u<3;u++) {
    gsm.GSM_send("AT\r\n");
    e = gsm.GSM_process("OK");
    if (e > 0) {
      gsm.pwr_off(1);
      delay(5000);
      break; 
    }
  }
  Serial.println("GSM ye");

  // HTTP stack on GSM
  http.init(gsm_buff, &gsm);
    
  last_status = now(); //0;
  last_upload = now();
    
  Serial.println("Entering main loop");
  delay(3000);
  //wdt_enable(WDTO_8S);
}

void loop() {
  switch(state) {
    case LOAD_CONFIG:
      cfg.load();
      cfg.load_files_count(0);
      cfg.load_files_count(1);
      config = cfg.get_config();
      if (config->http_status_time == 0)
	config->http_status_time = 10000;
      if (config->http_upload_time == 0)
	config->http_upload_time = 60000;
      state = IDLE;
      break;
    case IDLE: // General maintenance
      if (got_event) {
        lstate = state;
        state = EVENT;
        break;
      }
      else if ((now() - last_status) > (config->http_status_time)) {
        last_status = now();
        state = STATUS; 
      }
      else if ((now() - last_upload) > (config->http_upload_time)) {
         last_upload = now();
         state = UPLOAD;
      }
      else if ((millis() - last_measure) > (config->measure_length * 1000)) {
        lstate = state;
        start_measure = now();
        analog.pwr_on();
        state = MEASURE;
        break;
      }
      else if (lt != now()) {
        get_from_flash_P(PSTR("C: "), log_buff);    
        Serial.print(log_buff);
        Serial.println(((config->measure_length * 1000) - (millis() - last_measure))/1000, DEC);
        lt = now();
      }
      break;
    case MEASURE: // Collect measurements
      v = analog.read_all(); // Read all analog ports
      if (analog.get_all()) { // || (now() - start_measure) > 60) {
        last_measure = millis();
        analog.time_log_line(log_buff);   
        if (sd.is_available() < 0)
          sd.init();
	wdt_reset();
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
        //analog.pwr_off();
     	wdt_reset(); 
        if (initial_start < 3) {
          gsm.pwr_on();
          gsm.wake_modem();
          gsm.SMS_send("+4527148803", log_buff, strlen(log_buff));
          gsm.pwr_off();
          initial_start++; 
        }
        state = IDLE;   
        analog.reset();  
      } 
      if (got_event) {
        lstate = state;
        state = EVENT;
      }
      break;
    case STATUS: // Report status to server
      strcpy_P(log_buff, PSTR("http://dl2.zsuatt.com/status.php"));
      strcat_P(log_buff, PSTR("?id="));
      fmtUnsigned(config->id, smallbuff, 10);
      strcat(log_buff, smallbuff);
      strcat_P(log_buff, PSTR("&gl="));
      strcat(log_buff, gsm.GSM_get_lac());
      strcat_P(log_buff, PSTR("&gi="));
      strcat(log_buff, gsm.GSM_get_ci());
      strcat_P(log_buff, PSTR("&t="));
      fmtUnsigned(analog_values[15], smallbuff, 12);
      strcat(log_buff, smallbuff);
      strcat_P(log_buff, PSTR("&ts="));
      fmtUnsigned(now(), smallbuff, 12);
      strcat(log_buff, smallbuff);
      strcat_P(log_buff, PSTR("&u="));
      fmtUnsigned(millis(), smallbuff, 12);
      strcat(log_buff, smallbuff);
      u = sd.get_files_count(DATALOG);
      strcat_P(log_buff, PSTR("&cl="));
      fmtUnsigned(u, smallbuff, 12);
      strcat(log_buff, smallbuff);
      strcat_P(log_buff, PSTR("&cls=0"));
      v = http.GET(log_buff);
      
      get_from_flash_P(PSTR("R: "), log_buff);
      Serial.print(log_buff);
      Serial.println(v, DEC);
      gsm.pwr_off(); // Power off module
      
      // Syncronise RTC to server time
      if (RTC.get() < (now()-360) || RTC.get() > (now()+360)) {
        get_from_flash_P(PSTR("RTCB"), log_buff);
        Serial.println(log_buff);
        RTC.set(now());
      }

      state = lstate; // Done with reporting move to IDLE
      break;
    case UPLOAD: // Upload our files
      sd.increment_file(DATALOG);
      cfg.save_files_count(0);
      
      u = sd.get_files_count(DATALOG);
      if ((u -(sd.get_saved_count(DATALOG)+1)) > 5)
        p = u - 5;
      else
        p = sd.get_saved_count(DATALOG) + 1;
        
      for(uint16_t i = p; i < u; i++) {
        sd.set_files_count(DATALOG, i);
        Serial.print("UL #");
        Serial.println(i, DEC);
        v = upload_file(DATALOG);
        get_from_flash_P(PSTR("RET: "), log_buff);
        Serial.print(log_buff);
        Serial.println(v, DEC);
        if (v == 1) {
          sd.set_saved_count(DATALOG, i);
          cfg.save_files_count(1); 
        }
        wdt_reset(); // Feed the dog
      }
      gsm.pwr_off();
      cfg.load_files_count(0);
      state = lstate; // Done with upload move to IDLE
      break;
    case EVENT: // Lets handle the event
      if (got_event) {
        got_event = 0;
        analog.event_log_line(log_buff);
        if (sd.is_available() < 0)
            sd.init();
        filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
        sd.write(DATALOG, log_buff);
        sd.close(DATALOG);
        Serial.print(log_buff);
      }
      state = lstate;
      break;
  }
  if (Serial.available()) {
    v = Serial.read();
    if (v == 'r') {
      get_from_flash_P(PSTR("RFC!"), log_buff);
      Serial.println(log_buff);
      sd.reset_files_count();
      cfg.save_files_count(0);
    } else if (v == 't') {
      get_from_flash_P(PSTR("RSC!"), log_buff);
      Serial.println(log_buff);
      sd.reset_saved_count();
      cfg.save_files_count(1);
    } else if (v == 's') {
      get_from_flash_P(PSTR("FS!"), log_buff);
      Serial.println(log_buff); 
      state = STATUS; 
    } else if (v == 'u') {
      get_from_flash_P(PSTR("FFU!"), log_buff);
      Serial.println(log_buff);
      state = UPLOAD; 
    } else if (v == 'h') {
      Serial.print("hi ");
      Serial.println(state, DEC);
    } /*else if (v == 'l') {
      get_from_flash_P(PSTR("LFC!"), log_buff);
      Serial.println(log_buff);
      cfg.load_files_count(0); 
    } else if (v == 'i') {
      get_from_flash_P(PSTR("IFC!"), log_buff);
      Serial.println(log_buff);
      sd.increment_file(DATALOG);
      cfg.save_files_count(0);
    } */
  }
  wdt_reset(); 
  v = gsm.GSM_event_handler();
  if (v == GSM_EVENT_STATUS_REQ) {
	state = STATUS;
  }
}
