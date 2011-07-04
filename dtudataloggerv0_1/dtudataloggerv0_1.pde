#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <SdFat.h>
//#include <SD.h>
#include <NewSoftSerial.h>
#include <string.h>
#include <Wire.h>

//#define DEBUG 1
#undef DEBUG
#define SD_DEBUG 1

// States
#define SD_INIT 1
#define GSM_INIT 2
#define GPRS_INIT 3
#define IDLE 4
#define MEASURE 5
#define NTPDATE 6

// Files
#define SD_LOG 0
#define SD_SYSTEM 1
#define SD_CONF 2
#define SD_EVENT 3

// Default vaules
#define BUFFSIZE 50
const int NTP_PACKET_SIZE= 48;
boolean debug_on = true;
boolean has_gsm = true;
boolean sd_initialized = false;

prog_char string_0[] PROGMEM = "*** Starting up: DataLogger v1.1\n";
prog_char string_1[] PROGMEM = "Uptime: ";
prog_char string_2[] PROGMEM = " days and ";
prog_char string_3[] PROGMEM = ":";
prog_char string_4[] PROGMEM = "Free mem: ";
prog_char string_5[] PROGMEM = " State: ";
prog_char string_6[] PROGMEM = "Initializing SD card...";
prog_char string_7[] PROGMEM = "SD Init Failed.";
prog_char string_8[] PROGMEM = "SD Init Done!";
prog_char string_9[] PROGMEM = "Initializing GSM module...";
prog_char string_10[] PROGMEM = "GSM Module initialized!";
prog_char string_11[] PROGMEM = "GSM Init Failed.";
prog_char string_12[] PROGMEM = "Initializing GPRS module...";
PROGMEM const char *string_table[] = { string_0, string_1, string_2, 
                                       string_3, string_4, string_5,
                                       string_6, string_7, string_8,
                                       string_9, string_10, string_11,
                                       string_12 };

char state = 1;
NewSoftSerial gsmSerial(7, 8);
char recv_buffer[BUFFSIZE];
char *currptr = recv_buffer;
long currentmillis=0;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
int clockAddress = 0x68;
unsigned short analogvals[16];
int minv = 1024, maxv = 0, measurecnt = 0;
int i = 0, r = 0, lr = 0, c = 0, err = 0;
//short ptr[100];
float currtemp = 0;
//File lFile;
boolean gsm_ret = false;
extern unsigned int __bss_end;
extern void *__brkval;
char gsm_ci[5] = {0,0,0,0,0};
char gsm_lac[5] = {0,0,0,0,0};
char printbuff[50]; // Buffer for printing strings!! DONT TOUCH!!!
volatile boolean f_wdt=1;
unsigned long time = 0;
unsigned long fs = 0;

int strcmp_flash(char *str, void *ptr) {
  get_from_flash(ptr);
  return strncmp(str, printbuff, strlen(printbuff));
}

void get_from_flash(void *ptr) {
  strcpy_P(printbuff, (prog_char *)pgm_read_word(ptr));
}

void get_from_flash_P(const prog_char *ptr) {
  strcpy_P(printbuff, ptr); 
}

void print_from_flash(void *ptr) {
  get_from_flash(ptr);
  Serial.print(printbuff); 
}

void print_from_flash_P(const prog_char *ptr) {
  get_from_flash_P(ptr);
  Serial.print(printbuff);
}

int get_free_memory()
{
  int free_memory;
  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}

// *** UPTIME ***
void uptime()
{
  long days=0;
  long hours=0;
  long mins=0;
  long secs=0;
  secs = millis()/1000; //convect milliseconds to seconds
  mins=secs/60; //convert seconds to minutes
  hours=mins/60; //convert minutes to hours
  days=hours/24; //convert hours to days
  secs=secs-(mins*60); //subtract the coverted seconds to minutes in order to display 59 secs max
  mins=mins-(hours*60); //subtract the coverted minutes to hours in order to display 59 minutes max
  hours=hours-(days*24); //subtract the coverted hours to days in order to display 23 hours max
  //Display results
  Serial.println(' ');
  print_from_flash(&(string_table[1]));
    if (days>0) // days will displayed only if value is greater than zero
  {
    Serial.print(days);
    print_from_flash(&(string_table[2]));
    print_from_flash(&(string_table[3]));
  }
  Serial.print(hours);
  print_from_flash(&(string_table[3]));
  Serial.print(mins);
  print_from_flash(&(string_table[3]));
  Serial.println(secs);
}

// Rest of the code
void setup() {
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(14, INPUT);
  digitalWrite(14, LOW);
  Analog_init();
  Analog_setPin(0, 0);
  Analog_setPin(2, 0);
  Serial.begin(9600);
  gsmSerial.begin(19200); // GSM modem baud rate
  //wdt_enable(WDTO_8S);
  print_from_flash(&(string_table[0]));
  setup_watchdog(9);
}

void debug(char *txt) {
  if (debug_on) {
    Serial.println(txt);
    //if (sd_initialized) {
    //  File logFile = SD.open("system.log", FILE_WRITE);
    //  if (logFile) {
    //    logFile.println(txt);
    //    logFile.close();
    //  }
    //}
  }
}

void open_log() {
//  lFile = SD.open("data.log", FILE_WRITE);
}

void close_log() {
//  if (lFile)
//      lFile.close();
}
  
void save_to_log(float value) {
//    if (lFile) {
//      lFile.print(millis());
//      lFile.print(" ");
//      lFile.println(value);
//    }
}

void loop() {
  if (f_wdt==1) // wait for timed out watchdog
    f_wdt=0;
#ifdef DEBUG
  print_from_flash(&(string_table[4]));
  Serial.print(get_free_memory());
  print_from_flash(&(string_table[5]));
  Serial.println(state, HEX);
#endif
  //wdt_reset();
  switch(state) {
    case SD_INIT:
         get_from_flash(&(string_table[6]));
         debug(printbuff);
         if (!SD_init()) { //(!SD.begin(10)) {
             get_from_flash(&(string_table[7]));
             debug(printbuff);
             delay(1000);
         } else {
             get_from_flash(&(string_table[8]));
             debug(printbuff);
             sd_initialized = true;
             delay(500);
             if (has_gsm)
               state = GSM_INIT;
             else
               state = MEASURE; // IDLE
         }
         break;
    case GSM_INIT:
         get_from_flash(&(string_table[9]));
         debug(printbuff);
         gsm_ret = GSM_init();
         if (gsm_ret) {
           state = GPRS_INIT;
           get_from_flash(&(string_table[10]));
         } else
            get_from_flash(&(string_table[11]));
         debug(printbuff);
          
         break;
    case GPRS_INIT:
         get_from_flash(&(string_table[12]));
         debug(printbuff);
         GPRS_init();
         state = MEASURE;
         break;
    case IDLE:
         //debug("IDLING...");
         //uptime();
         //if (Serial.available()) {
         //  gsmSerial.print((unsigned char)Serial.read()); 
         //}
         //if (gsmSerial.available()) {
         //  Serial.print((unsigned char)gsmSerial.read());
         //}
         
         //minv = 1024;
         //maxv = 0;
         //for(i=0;i<100;i++) {
         //  Analog_readAll();
         //  ptr[i] = analogvals[15]; 
         //}
         
         //r = 0;
         //lr = 0;
         //c = 0;
         //for(i=0;i<(100-3);i++) {
         //  lr = r;
         //  if (ptr[i] < minv)
         //    minv = ptr[i];
         //  if (ptr[i] > maxv)
         //    maxv = ptr[i];
         //  if (ptr[i+3] < ptr[i+2] && ptr[i+2] < ptr[i+1] && ptr[i+1] < ptr[i])
         //    r = 1;
         //  else r = 0;
         //  if (r == 1 && lr == 0)
         //    c++;
         //}
         //Serial.print(minv);
         //Serial.print("\t");
        // Serial.print(maxv);
         //Serial.print("\t");
         //Serial.println(c);
         //Serial.println(Analog_read(15));
         //for(i=0;i<16;i++) {
         //  Serial.print(i);
         //  Serial.print("\t ");
         //  Serial.println(analogvals[i]); 
         //}
         //GSM_get_local_time();
         
         Analog_readAll();
         currtemp = getTemp();
         print_from_flash_P(PSTR("Temperature: "));
         Serial.println(currtemp);
 
         time = millis();
         SD_open_file(SD_EVENT, O_WRITE | O_CREAT | O_APPEND);
         SD_write_file_ulong(SD_EVENT, millis());
         get_from_flash_P(PSTR(" "));
         SD_write_file(SD_EVENT, printbuff);
         SD_write_file_float(SD_EVENT, currtemp);
         get_from_flash_P(PSTR("\n"));
         SD_write_file(SD_EVENT, printbuff);
         SD_close_file(SD_EVENT);
         time = millis() - time;
         print_from_flash_P(PSTR("SD latency: "));
         Serial.println(time);
             
         if (has_gsm) {
           GSM_request_net_status();
           print_from_flash_P(PSTR("GSM Lac: "));
           Serial.print(gsm_lac);
           print_from_flash_P(PSTR(" GSM Ci: "));
           Serial.println(gsm_ci);

           err = GPRS_check_state();
           Serial.println(err);
           if (err < 0) {
              state = GPRS_INIT;
              break; 
           }

           // Initialize TCP connection
           time = millis();
           gsm_ret= GPRS_connect("46.4.106.217", 80, true);
           if (gsm_ret) {
             print_from_flash_P(PSTR("Connection ok\n"));
             // Initialize sending        
             gsm_ret = GPRS_send_start();
             if (gsm_ret) {
               print_from_flash_P(PSTR("Send OK\n"));
               // Send request
               GPRS_send("GET /savetemp.php?temp=");
               GPRS_send_float(currtemp);
               GPRS_send("&gsmlac=");
               GPRS_send(gsm_lac);
               GPRS_send("&gsmci=");
               GPRS_send(gsm_ci);
               GPRS_send("&millis=");
               GPRS_send_ulong(millis());
               GPRS_send(" HTTP/1.0\r\n\r\n");
               gsm_ret = GPRS_send_end();
               if (gsm_ret)
                 print_from_flash_P(PSTR("End OK\n"));
             } else {
               print_from_flash_P(PSTR("Send failed\n"));
             }
           } else {
             print_from_flash_P(PSTR("Connect failed\n"));
           }
           // Close TCP connection
           gsm_ret = GPRS_close();
           if (gsm_ret)
             print_from_flash_P(PSTR("Close OK\n"));
           time = millis() - time;
           print_from_flash_P(PSTR("GSM latency: "));
           Serial.println(time);
           //delay(1000);      
           //system_sleep();
         }
         state = MEASURE;
         measurecnt = 50;
         break;
    case MEASURE:
         time = millis();
         if (measurecnt == 50)
           fs = SD_open_file(SD_LOG, O_WRITE | O_CREAT | O_APPEND);
           
         if (measurecnt > 0) {
           Analog_readAll();
           //Serial.print("File size: ");
           //Serial.println(fs);
           //Serial.print(millis());
           //Serial.print(" : ");
           SD_write_file_ulong(SD_LOG, millis());
           //get_from_flash_P(PSTR(" "));
           //SD_write_file(SD_LOG, printbuff);
           for(int g=0;g<16;g++) {
             SD_write_file_ushort(SD_LOG, analogvals[g]);
             //SD_write_file(SD_LOG, " ");
             //Serial.print(analogvals[g]);
             //Serial.print(" ");
           }         
           //Serial.print("\n");
           //get_from_flash_P(PSTR("\n"));
           SD_write_file(SD_LOG, "\n");
           time = millis() - time;
           print_from_flash_P(PSTR("Measurement latency: "));
           Serial.println(time);
           measurecnt--;
         } else {
           SD_close_file(SD_LOG);
           state = IDLE;
         }
         break;
    case NTPDATE:
         print_from_flash_P(PSTR("Sending NTP reqest\n"));
         time = millis();
         err = GPRS_check_state();
         Serial.println(err);
         if (err < 0) {
              state = GPRS_INIT;
              break; 
         }
         gsm_ret= GPRS_connect("46.4.106.217", 123, false);
         if (gsm_ret) {
           print_from_flash_P(PSTR("Connection ok\n"));
           // Initialize sending        
           gsm_ret = GPRS_send_start();
           if (gsm_ret) {
             print_from_flash_P(PSTR("Send OK\n"));
             // Send request
             memset(printbuff, 0, NTP_PACKET_SIZE);
             printbuff[0] = 0b11100011;   // LI, Version, Mode
             printbuff[1] = 0;     // Stratum, or type of clock
             printbuff[2] = 6;     // Polling Interval
             printbuff[3] = 0xEC;  // Peer Clock Precision
             // 8 bytes of zero for Root Delay & Root Dispersion
             printbuff[12]  = 49;
             printbuff[13]  = 0x4E;
             printbuff[14]  = 49;
             printbuff[15]  = 52;
             GPRS_send_raw(printbuff, NTP_PACKET_SIZE);
             gsm_ret = GPRS_send_end();
             if (gsm_ret)
               print_from_flash_P(PSTR("End OK\n"));
             } else {
               print_from_flash_P(PSTR("Send failed\n"));
             }
         } else {
             print_from_flash_P(PSTR("Connect failed\n"));
         }
         // Close TCP connection
         gsm_ret = GPRS_close();
         if (gsm_ret)
           print_from_flash_P(PSTR("Close OK\n"));
         time = millis() - time;
         print_from_flash_P(PSTR("NTP latency: "));
         Serial.println(time);    
         delay(5000);
         //state = IDLE;
         break;
    default:
         break;
  }

}
