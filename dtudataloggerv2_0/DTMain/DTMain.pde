#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <NewSoftSerial.h>
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

#define MEASURE_TIME 60000  //(5*60*1000ms) 

// Config
Config dlconfig;
DLConfig cfg;

// IO setup
DLAnalog analog(17, 16, 5, 4, 6, 14, LOW);
volatile uint16_t analog_values[16] = { 0 };
volatile uint32_t std_dev[16] = { 0 };
volatile uint8_t got_event = 0;
long last_measure = millis();
// SD setup
DLSD sd(true, 10); // Setup FULL_SPEED SPI and CS on pin 10
unsigned long filesize = 0;

// GSM
#define GSM_BUFF_SIZE 100
DLGSM gsm;

// HTTP
DLHTTP http;

#define LOG_BUFF_SIZE 100
time_t lt = now();
uint8_t state = LOAD_CONFIG; // lets start at IDLE state
char log_buff[LOG_BUFF_SIZE];
char gsm_buff[GSM_BUFF_SIZE];
uint8_t v;

void intnow() {
  analog.read_all(1);
  got_event = 1;
}

void setup() {
  wdt_disable(); // Disable watchdog
  Serial.begin(57600); // Initialize serial
  get_from_flash_P(PSTR("** Starting DLMain v2.1.0 (Jul 23, 2011)"), log_buff);  // Print welcome
  Serial.println(log_buff);
 
  get_from_flash_P(PSTR("Memory test: "), log_buff);
  Serial.print(log_buff);
  Serial.println(memory_test());
  
  setSyncProvider(RTC.get); // Setup time provider to RTC
  if(timeStatus()!= timeSet) { 
    get_from_flash_P(PSTR("** Unable to sync with the RTC"), log_buff);
  } else {
    get_from_flash_P(PSTR("** RTC has set the system time"), log_buff);
  }
  Serial.println(log_buff); 
  
  // SD init
  sd.debug(1); // No debugging
  v = sd.init();
  if (v == 1)
    get_from_flash_P(PSTR("** SD init successful!"), log_buff);
  else
    get_from_flash_P(PSTR("** SD init failed!"), log_buff);
  Serial.println(log_buff);
  // AT this point we proceed, but RTC/SD might be broken...
  
  // Config file loading
  cfg.init(&dlconfig, &sd, &analog, log_buff, LOG_BUFF_SIZE);
  
  analog.init(analog_values, std_dev); // Initialize IO with buffers
  analog.debug(1); // Turn on analog debug

  analog.set_pin(0, EVENT);
  //analog.set_pin(1, ANALOG);
  //analog.set_pin(2, ANALOG);
  //analog.set_pin(3, ANALOG);
  //analog.set_pin(4, ANALOG);
  analog.set_pin(5, COUNTER);
  //analog.set_pin(6, ANALOG);
  analog.set_pin(7, ANALOG);
  analog.set_pin(8, COUNTER);
  //analog.set_pin(9, ANALOG);
  //analog.set_pin(10, ANALOG);
  analog.set_pin(11, DIGITAL);
  
  //analog.set_pin(2, DIGITAL);
  //analog.set_pin(7, ANALOG);
  //analog.set_pin(8, DIGITAL);
  //analog.set_pin(9, ANALOG);
  //analog.set_pin(13, ANALOG);

  attachInterrupt(0, intnow, CHANGE);
  
  // Initialize GSM
  gsm.init(gsm_buff, GSM_BUFF_SIZE, 5);
  gsm.debug(1);
  // HTTP stack on GSM
  http.init(gsm_buff, &gsm);
    
  wdt_enable(WDTO_8S);
}

void loop() {
  switch(state) {
    case LOAD_CONFIG:
      cfg.load();
      state = IDLE;
      break;
    case IDLE: // General maintenance
      if (got_event) {
        state = EVENT;
        break;
      }
      if ((millis() - last_measure) > MEASURE_TIME) {
        state = MEASURE;
        break;
      }
      if (lt != now()) {
        get_from_flash_P(PSTR("Time until next measurement: "), log_buff);    
        Serial.print(log_buff);
        Serial.println((MEASURE_TIME - (millis() - last_measure))/1000, DEC);
        lt = now();
      }
      break;
    case MEASURE: // Collect measurements
      v = analog.read_all(); // Read all analog ports
      if (v >= 10) {
        if (analog.get_all()) {
          last_measure = millis();
          analog.time_log_line(log_buff);   
          if (sd.is_available() < 0)
            sd.init();
          filesize = sd.open(DATALOG, O_RDWR | O_CREAT | O_APPEND);
          Serial.print(log_buff);
          sd.write(DATALOG, log_buff);
          sd.close(DATALOG);
          state = IDLE;   
        }
        analog.reset();
      }
      if (got_event)
        state = EVENT;
      break;
    case STATUS: // Report status to server
      strcpy_P(log_buff, PSTR("http://attila.patup.com/test.php?id=1"));
      v = http.GET(log_buff);
      Serial.println(v);
      state = IDLE; // Done with reporting move to IDLE
      break;
    case UPLOAD: // Upload our files
      state = IDLE; // Done with upload move to IDLE
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
      state = STATUS;
      break;
  }
  wdt_reset(); 
}
