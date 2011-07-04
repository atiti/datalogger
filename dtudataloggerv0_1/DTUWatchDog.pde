#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= ~_BV(bit))
#endif

// *** SLEEP/WATCHDOG
void system_sleep() {
  //cbi(ADCSRA,ADEN); // Disable ADCs
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set powerdown sleep mode
  sleep_enable();
  sleep_mode(); // Sleeeep
  sleep_disable(); // Execution continues after it resumes
  //sbi(ADCSRA, ADEN); // Enable ADCs
}

void setup_watchdog(int ii) {
  byte bb;
  int ww;
  if (ii > 9) ii = 0;
  bb = ii & 7;
  if (ii > 7) bb |= (1<<5);
  bb |= (1 <<WDCE);
  ww = bb;
   
  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

ISR(WDT_vect) {
  f_wdt=1;
}
// *** END OF SLEEP/WATCHDOG ***

