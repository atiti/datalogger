Sd2Card card;
SdVolume vol;
SdFile root;
SdFile files[4];
boolean files_open[4] = {false, false, false, false};
prog_char sd_filename_0[] PROGMEM = "DATALOG.TXT";
prog_char sd_filename_1[] PROGMEM = "SYSTEM.TXT";
prog_char sd_filename_2[] PROGMEM = "CONFIG.TXT";
prog_char sd_filename_3[] PROGMEM = "EVENT.TXT";

PROGMEM const char *sd_filename_table[] = { sd_filename_0, sd_filename_1, sd_filename_2, 
                                           sd_filename_3 }; 
                                           
boolean SD_init() {
  uint8_t ret;
#ifdef SD_DEBUG
  int t = millis();
#endif
  ret = card.init(SPI_FULL_SPEED, 10); 
#ifdef SD_DEBUG
  Serial.print("card.init ret: ");
  Serial.println(ret,DEC);
#endif
  if (!ret)
    return false;
 
  if (!vol.init(&card))
    return false;
 
  if (!root.openRoot(&vol))
    return false;
#ifdef SD_DEBUG
  if (debug_on) {
    t = millis() - t;
    print_from_flash_P(PSTR("Init time: "));
    Serial.println(t);
    print_from_flash_P(PSTR("Volume is FAT"));
    Serial.println(vol.fatType(),DEC);
    
  }
#endif
  return true;
}

unsigned long SD_open_file(uint8_t n, uint8_t flags) {
  uint8_t ret;
  unsigned long fsize = 0;
  digitalWrite(10, LOW);
  for(int j=0;j<4;j++) {
    if (files_open[j] == true && j != n) {
#ifdef SD_DEBUG
      if (debug_on) {
        Serial.print("Closing file #");
        Serial.println(j,DEC);
      }
#endif
      SD_close_file(n);
    } 
  }
  if (files_open[n] == false) {
    get_from_flash(&(sd_filename_table[n]));
    ret = files[n].open(&root, printbuff, flags);
    files_open[n] = true;
    fsize = files[n].fileSize();
  }
#ifdef SD_DEBUG
  if (debug_on && ret) {
    Serial.print("Opened file #");
    Serial.println(n,DEC); 
  }
#endif
  return fsize;
}

boolean SD_close_file(uint8_t n) {
  files[n].sync();
  if (!files[n].close())
    return false;
  files_open[n] = false;
#ifdef SD_DEBUG
  Serial.print("Closed file #");
  Serial.println(n,DEC);
#endif
  digitalWrite(10, HIGH);
  return true;
}

void SD_write_file(uint8_t n, char *ptr) {
  files[n].writeError = false;
  files[n].print(ptr);
  files[n].sync();
#ifdef SD_DEBUG
  if (files[n].writeError) {
    Serial.println("Writing failed to file!"); 
  }
#endif  
}

void SD_write_file_float(uint8_t n, float a) {
  files[n].writeError = false;
  files[n].print(a);
  files[n].sync();
#ifdef SD_DEBUG
  if (files[n].writeError) {
    Serial.println("Writing failed to file!"); 
  }
#endif
}
void SD_write_file_ushort(uint8_t n, unsigned short a) {
  files[n].writeError = false;
  files[n].print(a);
  files[n].sync();
#ifdef SD_DEBUG
  if (files[n].writeError) {
    Serial.println("Writing failed to file!"); 
  }
#endif
}
void SD_write_file_int(uint8_t n, int a) {
  files[n].writeError = false;
  files[n].print(a);
  files[n].sync();
#ifdef SD_DEBUG
  if (files[n].writeError) {
   Serial.println("Writing failed to file!"); 
  }
#endif
}

void SD_write_file_ulong(uint8_t n, unsigned long a) {
  files[n].writeError = false;
  files[n].print(a);
  files[n].sync();
#ifdef SD_DEBUG
  if (files[n].writeError) {
   Serial.println("Writing failed to file!"); 
  }
#endif
}
