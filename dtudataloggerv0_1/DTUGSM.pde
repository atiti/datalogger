prog_char gsm_string_0[] PROGMEM = "\r\nATE0\r\n";
prog_char gsm_string_1[] PROGMEM = "AT+GOI\r\n";
prog_char gsm_string_2[] PROGMEM = "AT+CREG=2\r\n";
prog_char gsm_string_3[] PROGMEM = "AT+CGREG=2\r\n";
prog_char gsm_string_4[] PROGMEM = "AT+CGREG?\r\n";
prog_char gsm_string_5[] PROGMEM = "AT+CGATT?\r\n";
prog_char gsm_string_6[] PROGMEM = "AT+CGATT=1\r\n";
prog_char gsm_string_7[] PROGMEM = "AT+CIPCSGP=1,\"internet.bibob.dk\"\r\n";
prog_char gsm_string_8[] PROGMEM = "AT+CLPORT=\"TCP\",\"2020\"\r\n";
prog_char gsm_string_9[] PROGMEM = "AT+CSTT=\"internet.bibob.dk\",\"\",\"\"\r\n";
prog_char gsm_string_10[] PROGMEM = "AT+CIPSRIP=1\r\n";
prog_char gsm_string_11[] PROGMEM = "AT+CIICR\r\n";
prog_char gsm_string_12[] PROGMEM = "AT+CIFSR\r\n";
prog_char gsm_string_13[] PROGMEM = "AT+CIPSTART=\"TCP\",\"";
prog_char gsm_string_14[] PROGMEM = "AT+CIPSTART=\"UDP\",\"";
prog_char gsm_string_15[] PROGMEM = "AT+CIPSEND\r\n";
prog_char gsm_string_16[] PROGMEM = "AT+CIPCLOSE\r\n";
prog_char gsm_string_17[] PROGMEM = "AT+CIPSTATUS\r\n";
prog_char gsm_string_18[] PROGMEM = "AT+CLTS\r\n";
PROGMEM const char *gsm_string_table[] = { gsm_string_0, gsm_string_1, gsm_string_2, 
                                           gsm_string_3, gsm_string_4, gsm_string_5, 
                                           gsm_string_6, gsm_string_7, gsm_string_8,
                                           gsm_string_9, gsm_string_10, gsm_string_11,
                                           gsm_string_12, gsm_string_13, gsm_string_14,
                                           gsm_string_15, gsm_string_16, gsm_string_17,
                                           gsm_string_18 };

prog_char gprs_state_0[] PROGMEM = "STATE:";
prog_char gprs_state_1[] PROGMEM = "PDP";
prog_char gprs_state_2[] PROGMEM = "CONNECT";
prog_char gprs_state_3[] PROGMEM = "TCP CONNECT";
prog_char gprs_state_4[] PROGMEM = "UDP CONNECT";
prog_char gprs_state_5[] PROGMEM = "TCP CLOSED";
prog_char gprs_state_6[] PROGMEM = "IP STATUS";
prog_char gprs_state_7[] PROGMEM = "UDP CLOSED";
prog_char gprs_state_8[] PROGMEM = "";
prog_char gprs_state_9[] PROGMEM = "";

PROGMEM const char *gprs_state_table[] = { gprs_state_0, gprs_state_1, gprs_state_2,
                                           gprs_state_3, gprs_state_4, gprs_state_5,
                                           gprs_state_6, gprs_state_7, gprs_state_8};


// Receive full lines which fit in the buffer
void GSM_recvcmd(char *ptr, int len) {
  char curchar[2];
  int i = 0;
  // Read the first 2 bytes (should be the beginning of a reply)
  for(i=0;i<2;i++) {
    curchar[i] = gsmSerial.read();
  }
  // Check that we got the top of the reply
  if (curchar[0] != '\r' && curchar[1] != '\n') {
    ptr[0] = curchar[0];
    ptr[1] = curchar[1];
    i = 2;
  } else {
    i = 0;
  }
  // Read the rest of the line
  while (curchar[0] != '\n' && i < len) {
    if (gsmSerial.available()) {
      curchar[0] = (char)gsmSerial.read();
      ptr[i] = curchar[0];
      i++;
    } else {
      ptr[i] = 0;
      break;
    }
  }
  ptr[i] = 0;
}

void GSM_send(char *msg) {
  gsmSerial.print(msg);
#ifdef DEBUG
  Serial.print(msg);
#endif
}
void GSM_send_num(int v) {
  gsmSerial.print(v);
#ifdef DEBUG
  Serial.print(v);
#endif
}
void GSM_send_raw(char b) {
  gsmSerial.print(b, BYTE); 
  Serial.print(b, HEX);
  Serial.print(" ");
}

// Process incoming messages with a timeout of 500ms
boolean GSM_process(char *check, uint8_t tout = 5) {
  int i = 0, a = 0, nr = tout;
  boolean ret = false;
  do {
    a = gsmSerial.available();
    if (a) {
      nr = tout;
      GSM_recvcmd(recv_buffer, BUFFSIZE);
#ifdef DEBUG
      Serial.print(i);
      Serial.print(": ");
      Serial.print(recv_buffer);
#endif
      if (check != NULL) {
        if (strncmp(recv_buffer, check, strlen(check)) == 0)
          ret = true; 
      }
      if (strncmp(recv_buffer, "+CGREG:", 7) == 0) {
        strncpy(gsm_lac, recv_buffer+13, 4);
        strncpy(gsm_ci, recv_buffer+20, 4);        
      }
      if (strncmp(recv_buffer, "+CREG:", 6) == 0) {
        strncpy(gsm_lac, recv_buffer+12, 4);
        strncpy(gsm_ci, recv_buffer+19, 4);        
      }
      i++;
    } else {
      nr--;
      delay(100);
    }
  } while (a || nr > 0);
  return ret;
}

boolean GSM_init() {
  get_from_flash(&(gsm_string_table[0])); // Fetch AT echo turnoff command
  GSM_send(printbuff);
  gsm_ret = GSM_process("OK");
  if (gsm_ret == true)
    Serial.println("Echo now off!");
  else {
    return false;
  }
  get_from_flash(&(gsm_string_table[1])); // Fetch init AT+GOI command
  GSM_send(printbuff);
  gsm_ret = GSM_process("SIM900");
  if (gsm_ret == true)
    Serial.println("SIM900 detected!");
  else {
    return false;
  } 
  get_from_flash(&(gsm_string_table[2])); // Turn on network registration verbose
  GSM_send(printbuff);
  GSM_process(NULL);
  return true;
}

void GPRS_init() {
  get_from_flash(&(gsm_string_table[3])); // Turn on GPRS registration verbose (AT+CGREG)
  GSM_send(printbuff);
  GSM_process(NULL);
  
  get_from_flash(&(gsm_string_table[4])); // Query GPRS registration status (AT+CGREG?)
  GSM_send(printbuff);
  GSM_process(NULL);
  
  get_from_flash(&(gsm_string_table[5])); // Query GPRS PDP context attach status? (AT+CGATT?)
  GSM_send(printbuff);
  GSM_process(NULL);
  
  get_from_flash(&(gsm_string_table[6])); // Attach GPRS context (AT+CGATT=1)
  GSM_send(printbuff);
  GSM_process(NULL);
  
  get_from_flash(&(gsm_string_table[7])); // Set GPRS for connection mode (AT+CIPCSGP)
  GSM_send(printbuff);
  GSM_process(NULL);

  get_from_flash(&(gsm_string_table[8])); // Set the local port for the TCP/IP stack
  GSM_send(printbuff);
  GSM_process(NULL);

  get_from_flash(&(gsm_string_table[9])); // Set APN, username and password
  GSM_send(printbuff);
  GSM_process(NULL);

  get_from_flash(&(gsm_string_table[10])); // Set displaying of IP address and port (AT+CIPSIRP)
  GSM_send(printbuff);
  GSM_process(NULL);

  get_from_flash(&(gsm_string_table[11])); // Bring up the GPRS connection (AT+CIICR)
  GSM_send(printbuff);
  GSM_process(NULL);
  
  get_from_flash(&(gsm_string_table[12])); // Get the local IP address (AT+CIFSR)
  GSM_send(printbuff);
  GSM_process(NULL);
}

void GSM_request_net_status() {
  get_from_flash(&(gsm_string_table[4]));
  GSM_send(printbuff);
  GSM_process(NULL);
}

void GSM_get_local_time() {
  get_from_flash(&(gsm_string_table[18]));
  GSM_send(printbuff);
  GSM_process(NULL);
}

boolean GPRS_connect(char *server, int port, boolean tcp) {
  if (tcp)
    get_from_flash(&(gsm_string_table[13]));
  else
    get_from_flash(&(gsm_string_table[14]));
  GSM_send(printbuff);
  
  GSM_send(server);
  GSM_send("\",\"");
  GSM_send_num(port);
  GSM_send("\"\r\n");
  return GSM_process("OK");
}

boolean GPRS_send_start() {
  int nr = 10;
  do {
    get_from_flash(&(gsm_string_table[15])); // Send AT+CIPSEND
    GSM_send(printbuff);
    delay(100);
    nr--;
  } while (GSM_process(">") == false && nr > 0);
  if (nr <= 0)
    return false;
  return true;
}

void GPRS_send(char *data) {
  GSM_send(data);
}

void GPRS_send_raw(char *data, int len) {
  for(i=0;i<len;i++) {
    Serial.print(i);
    Serial.println("");
    GSM_send(&data[i]);
  } 
}

void GPRS_send_float(float n) {
  gsmSerial.print(n);
}

void GPRS_send_ulong(unsigned long n) {
  gsmSerial.print(n);
}

boolean GPRS_send_end() {
  gsmSerial.print(26, BYTE);
  gsmSerial.print(byte(26));
  GSM_send("\r\n");
  return GSM_process("SEND OK");
}

boolean GPRS_close() {
  get_from_flash(&(gsm_string_table[16])); // Send AT+CIPCLOSE
  GSM_send(printbuff);
  return GSM_process("CLOSE OK");  
}

int GPRS_check_state() {
  get_from_flash(&(gsm_string_table[17]));
  GSM_send(printbuff);
  GSM_process(NULL);
  if (strcmp_flash(recv_buffer, &(gprs_state_table[0])) == 0) {
    if (strcmp_flash(recv_buffer+7, &(gprs_state_table[1])) == 0)
      return -2;
    else if (strcmp_flash(recv_buffer+7, &(gprs_state_table[2])) == 0)
      return 2;
    else if (strcmp_flash(recv_buffer+7, &(gprs_state_table[3])) == 0)
      return 3;
    else if (strcmp_flash(recv_buffer+7, &(gprs_state_table[4])) == 0)
      return 4;
    else if (strcmp_flash(recv_buffer+7, &(gprs_state_table[7])) == 0)
      return 5;
    else if (strcmp_flash(recv_buffer+7, &(gprs_state_table[5])) == 0)
      return 1;
    else if (strcmp_flash(recv_buffer+7, &(gprs_state_table[6])) == 0)
      return 0;
      
  } 
  return -1;
}

