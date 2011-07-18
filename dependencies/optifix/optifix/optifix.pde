// optifix.pde
// Jan 2011 by Bill Westfield ("WestfW")
//
// this sketch allows an Arduino to program a new version of
// the optiboot bootloader onto another Arduino Uno.  This is
// useful for fixing some of the bootloader bugs that have shown
// up since the Uno was shipped.
//
// This version of the bootloader fixes:
// timer initialization (also fixed in core)
// "30k limit" bug.
// "sketch amnesia" bug (Set R1 to zero.)
//
// It is based on AVRISP, and you wire up the target system as
// described here:  http://arduino.cc/en/Tutorial/ArduinoISP
//
// using the following pins:
// 10: slave reset
// 11: MOSI
// 12: MISO
// 13: SCK
// ----------------------------------------------------------------------

// The following credits are from AVRISP.  It turns out that there isn't
// a lot of AVRISP left in this sketch, but probably if AVRISP had never
// existed,  this sketch would not have been written.
//
// October 2009 by David A. Mellis
// - Added support for the read signature command
// 
// February 2009 by Randall Bohn
// - Added support for writing to EEPROM (what took so long?)
// Windows users should consider WinAVR's avrdude instead of the
// avrdude included with Arduino software.
//
// January 2008 by Randall Bohn
// - Thanks to Amplificar for helping me with the STK500 protocol
// - The AVRISP/STK500 (mk I) protocol is used in the arduino bootloader
// - The SPI functions herein were developed for the AVR910_ARD programmer 
// - More information at http://code.google.com/p/mega-isp

#include <avr/pgmspace.h>
#include "optiLoader.h"

/*
 * Pins to target
 */
#define SCK 13
#define MISO 12
#define MOSI 11
#define RESET 10
#define POWER 9

// STK Definitions; we can still use these as return codes
#define STK_OK 0x10
#define STK_FAILED 0x11


// Useful message printing definitions
#define fp(string) flashprint(PSTR(string));
#define debug(string) // flashprint(PSTR(string));
#define error(string) flashprint(PSTR(string));

// Forward references
void pulse(int pin, int times);
void read_image (image_t *ip);

// Global Variables

/*
 * Table of defined images
 */
image_t *images[] = {
  &image_328, 0
};

int pmode=0;
// address for reading and writing, set by 'U' command
int here;

uint16_t target_type = 0;	       /* type of target_cpu */
uint16_t target_startaddr;
byte target_pagesize;		       /* Page size for flash programming (bytes) */
uint8_t *buff;

image_t *target_flashptr;	       /* pointer to target info in flash */
byte target_code[512];		       /* The whole code */


void setup () {
  Serial.begin(19200);			/* Initialize serial for status msgs */
  pinMode(13, OUTPUT);			/* Blink the pin13 LED a few times */
  pulse(13,20);
}

void loop (void) {
  fp("\nOptiFix Bootstrap programmer.\n2011 by Bill Westfield (WestfW)\n\n");
    target_poweron();			/* Turn on target power */
    do {
	if (!target_identify())		/* Figure out what kind of CPU */
	    break;
	if (!target_findimage())	/* look for an image */
	    break;
	if (!target_progfuses())	/* get fuses ready to program */
	    break;
	if (!target_program())		/* Program the image */
	    break;
	(void) target_normfuses();	/* reset fuses to normal mode */
    } while (0);

    target_poweroff();			/* turn power off */

    fp ("\nType 'G' or hit RESET for next chip\n")
    while (1) {
	if (Serial.read() == 'G')
	    break;
    }
}

/*
 * Low level support functions
 */

/*
 * flashprint
 * print a text string direct from flash memory to Serial
 */
void flashprint (const char p[])
{
    byte c;
    while (0 != (c = pgm_read_byte(p++))) {
	Serial.write(c);
    }
}

/*
 * hexton
 * Turn a Hex digit (0..9, A..F) into the equivalent binary value (0-16)
 */
byte hexton (byte h)
{
  if (h >= '0' && h <= '9')
    return(h - '0');
  if (h >= 'A' && h <= 'F')
    return((h - 'A') + 10);
  error("Bad hex digit!");
}

/*
 * pulse
 * turn a pin on and off a few times; indicates life via LED
 */
#define PTIME 30
void pulse (int pin, int times) {
  do {
    digitalWrite(pin, HIGH);
    delay(PTIME);
    digitalWrite(pin, LOW);
    delay(PTIME);
  } 
  while (times--);
}

/*
 * spi_init
 * initialize the AVR SPI peripheral
 */
void spi_init () {
  uint8_t x;
  SPCR = 0x53;  // SPIE | MSTR | SPR1 | SPR0
  x=SPSR;
  x=SPDR;
}

/*
 * spi_wait
 * wait for SPI transfer to complete
 */
void spi_wait () {
  debug("spi_wait");
  do {
  } 
  while (!(SPSR & (1 << SPIF)));
}

/*
 * spi_send
 * send a byte via SPI, wait for the transfer.
 */
uint8_t spi_send (uint8_t b) {
  uint8_t reply;
  SPDR=b;
  spi_wait();
  reply = SPDR;
  return reply;
}


boolean target_identify ()
{
  boolean result;
  target_type = 0;
  fp("\nReading signature:");
  target_type = read_signature();
  if (target_type == 0 || target_type == 0xFFFF) {
    fp(" Bad value: ");
    result = false;
  } else {
    result = true;
  }
  Serial.println(target_type, HEX);
  if (target_type == 0) {
    fp("  (no target attached?)\n");
  }
  return result;
}

/*
 * read_image
 *
 * Read an intel hex image from a string in pgm memory.
 * We assume that the image does not exceed the 512 bytes that we have
 * allowed for it to have.  that would be bad.
 * Also read other data from the image, such as fuse and protecttion byte
 * values during programming, and for after we're done.
 */
void read_image (image_t *ip)
{
  uint16_t len, totlen=0, addr;
  char *hextext = &ip->image_hexcode[0];
  target_startaddr = 0;
  target_pagesize = pgm_read_byte(&ip->image_pagesize);
  byte b, cksum = 0;

  while (1) {
    if (pgm_read_byte(hextext++) != ':') {
      error("No colon");
      break;
    }
    len = hexton(pgm_read_byte(hextext++));
    len = (len<<4) + hexton(pgm_read_byte(hextext++));
    cksum = len;

    b = hexton(pgm_read_byte(hextext++)); /* record type */
    b = (b<<4) + hexton(pgm_read_byte(hextext++));
    cksum += b;
    addr = b;
    b = hexton(pgm_read_byte(hextext++)); /* record type */
    b = (b<<4) + hexton(pgm_read_byte(hextext++));
    cksum += b;
    addr = (addr << 8) + b;
    if (target_startaddr == 0) {
      target_startaddr = addr;
      fp("  Start address at ");
      Serial.println(addr, HEX);
    } else if (addr == 0) {
      break;
    }

    b = hexton(pgm_read_byte(hextext++)); /* record type */
    b = (b<<4) + hexton(pgm_read_byte(hextext++));
    cksum += b;

    for (byte i=0; i < len; i++) {
      b = hexton(pgm_read_byte(hextext++));
      b = (b<<4) + hexton(pgm_read_byte(hextext++));
      if (addr - target_startaddr >= sizeof(target_code)) {
	error("Code extends beyond allowed range");
	break;
      }
      target_code[addr++ - target_startaddr] = b;
      cksum += b;
#if VERBOSE
      Serial.print(b, HEX);
      Serial.write(' ');
#endif
      totlen++;
      if (totlen >= sizeof(target_code)) {
	  error("Too much code");
	  break;
      }
    }
    b = hexton(pgm_read_byte(hextext++)); /* checksum */
    b = (b<<4) + hexton(pgm_read_byte(hextext++));
    cksum += b;
    if (cksum != 0) {
      error("Bad checksum: ");
      Serial.print(cksum, HEX);
    }
    if (pgm_read_byte(hextext++) != '\n') {
      error("No end of line");
      break;
    }
#if VERBOSE
    Serial.println();
#endif
  }
  fp("  Total bytes read: ");
  Serial.println(totlen);
}

/*
 * target_findimage
 *
 * given target_type loaded with the relevant part of the device signature,
 * search the hex images that we have programmed in flash, looking for one
 * that matches.
 */

boolean target_findimage ()
{
  image_t *ip;
  fp("Searching for image...\n");
  for (byte i=0; i < sizeof(images)/sizeof(images[0]); i++) {
    target_flashptr = ip = images[i];
    if (ip && (pgm_read_word(&ip->image_chipsig) == target_type)) {
	fp("  Found \"");
	flashprint(&ip->image_name[0]);
	fp("\" for ");
	flashprint(&ip->image_chipname[0]);
	fp("\n");
	read_image(ip);
	return true;
    }
  }
  fp(" Not Found\n");
  return(false);
}

/*
 * target_progfuses
 * given initialized target image data, re-program the fuses to allow
 * the optiboot image to be programmed.
 */

boolean target_progfuses ()
{
  byte f;
  fp("\nSetting fuses for programming");

  f = pgm_read_byte(&target_flashptr->image_progfuses[FUSE_PROT]);
  if (f) {
    fp("\n  Lock: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xE0, 0x00, f), HEX);
  }
  f = pgm_read_byte(&target_flashptr->image_progfuses[FUSE_LOW]);
  if (f) {
    fp("  Low: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xA0, 0x00, f), HEX);
  }
  f = pgm_read_byte(&target_flashptr->image_progfuses[FUSE_HIGH]);
  if (f) {
    fp("  High: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xA8, 0x00, f), HEX);
  }
  f = pgm_read_byte(&target_flashptr->image_progfuses[FUSE_EXT]);
  if (f) {
    fp("  Ext: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xA4, 0x00, f), HEX);
  }
  Serial.println();
  return true;			/* */
}

/*
 * target_program
 * Actually program the image into the target chip
 */

boolean target_program ()
{
  int l;				/* actual length */

  fp("\nProgramming bootloader: ");
  here = target_startaddr>>1;		/* word address */
  buff = target_code;
  l = 512;
  Serial.print(l, DEC);
  fp(" bytes at 0x");
  Serial.println(here, HEX);
  
  spi_transaction(0xAC, 0x80, 0, 0);	/* chip erase */
  delay(1000);
  if (write_flash(l) != STK_OK) {
    error("\nFlash Write Failed");
    return false;
  }
  return true;			/*  */
}

/*
 * target_normfuses
 * reprogram the fuses to the state they should be in for bootloader
 * based programming
 */
boolean target_normfuses ()
{
  byte f;
  fp("\nRestoring normal fuses");

  f = pgm_read_byte(&target_flashptr->image_normfuses[FUSE_PROT]);
  if (f) {
    fp("\n  Lock: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xE0, 0x00, f), HEX);
  }
  f = pgm_read_byte(&target_flashptr->image_normfuses[FUSE_LOW]);
  if (f) {
    fp("  Low: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xA0, 0x00, f), HEX);
  }
  f = pgm_read_byte(&target_flashptr->image_normfuses[FUSE_HIGH]);
  if (f) {
    fp("  High: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xA8, 0x00, f), HEX);
  }
  f = pgm_read_byte(&target_flashptr->image_normfuses[FUSE_EXT]);
  if (f) {
    fp("  Ext: ");
    Serial.print(f, HEX);
    fp(" ");
    Serial.print(spi_transaction(0xAC, 0xA4, 0x00, f), HEX);
  }
  Serial.println();
  return true;			/* */
}

/*
 * target_poweron
 * Turn on power to the target chip (assuming that it is powered through
 * the relevant IO pin of THIS arduino.)
 */
boolean target_poweron ()
{
  digitalWrite(RESET, LOW);  // reset it right away.
  pinMode(RESET, OUTPUT);
  delay(200);
  fp("Starting Program Mode");
  start_pmode();
  fp(" [OK]\n");
  return true;
}

boolean target_poweroff ()
{
  end_pmode();
  return true;
}

unsigned long spi_transaction (uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint8_t n, m;
  spi_send(a); 
  n=spi_send(b);
  //if (n != a) error = -1;
  m=spi_send(c);
  return 0xFFFFFF & ((n<<16)+(m<<8) + spi_send(d));
}

void start_pmode () {
  pinMode(13, INPUT); // restore to default
  spi_init();
  debug("...spi_init done");
  // following delays may not work on all targets...
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);
  pinMode(SCK, OUTPUT);
  digitalWrite(SCK, LOW);
  delay(50);
  digitalWrite(RESET, LOW);
  delay(50);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  debug("...spi_transaction");
  spi_transaction(0xAC, 0x53, 0x00, 0x00);
  debug("...Done");
  pmode = 1;
}

void end_pmode () {
  SPCR = 0;				/* reset SPI */
  digitalWrite(MISO, 0);		/* Make sure pullups are off too */
  pinMode(MISO, INPUT);
  digitalWrite(MOSI, 0);
  pinMode(MOSI, INPUT);
  digitalWrite(SCK, 0);
  pinMode(SCK, INPUT);
  digitalWrite(RESET, 0);
  pinMode(RESET, INPUT);
  pmode = 0;
}

void flash (uint8_t hilo, int addr, uint8_t data) {
#if VERBOSE
  Serial.print(data, HEX);
  fp(":");
  Serial.print(spi_transaction(0x40+8*hilo, 
			       addr>>8 & 0xFF, 
			       addr & 0xFF,
			       data), HEX);
  fp(" ");
#else
  (void) spi_transaction(0x40+8*hilo, 
			       addr>>8 & 0xFF, 
			       addr & 0xFF,
			 data);
#endif
}

void commit (int addr) {
  fp("  Commit Page: ");
  Serial.print(addr, HEX);
  fp(":");
  Serial.println(spi_transaction(0x4C, (addr >> 8) & 0xFF, addr & 0xFF, 0), HEX);
  delay(100);
}

//#define _current_page(x) (here & 0xFFFFE0)
int current_page (int addr) {
  if (target_pagesize == 32) return here & 0xFFFFFFF0;
  if (target_pagesize == 64) return here & 0xFFFFFFE0;
  if (target_pagesize == 128) return here & 0xFFFFFFC0;
  return here;
}

uint8_t write_flash (int length) {
  if (target_pagesize < 1) return STK_FAILED;
  //if (target_pagesize != 64) return STK_FAILED;
  int page = current_page(here);
  int x = 0;
  while (x < length) {
    if (page != current_page(here)) {
      commit(page);
      page = current_page(here);
    }
    flash(LOW, here, buff[x]);
    flash(HIGH, here, buff[x+1]);
    x+=2;
    here++;
  }

  commit(page);

  return STK_OK;
}

uint16_t read_signature () {
  uint8_t sig_middle = spi_transaction(0x30, 0x00, 0x01, 0x00);
  uint8_t sig_low = spi_transaction(0x30, 0x00, 0x02, 0x00);
  return ((sig_middle << 8) + sig_low);
}

/*
 * Bootload images.
 * These are the intel Hex files produced by the optiboot makefile,
 * with a small amount of automatic editing to turn them into C strings,
 * and a header attched to identify them
 */

image_t PROGMEM image_328 = {
    {"optiboot_atmega328.hex"},
    {"atmega328"},
    0x950F,				/* Signature bytes for 328P */
    {0x3F,0xFF,0xDE,0x05,0},
    {0x0F,0,0,0,0},
    128,
    {
      ":107E0000112484B714BE81FFE6D085E08093810001\n"
      ":107E100082E08093C00088E18093C10086E0809377\n"
      ":107E2000C20080E18093C4008EE0CFD0259A86E026\n"
      ":107E300020E33CEF91E0309385002093840096BBD3\n"
      ":107E4000B09BFECF1D9AA8958150A9F7DD24D3944D\n"
      ":107E5000A5E0EA2EF1E1FF2EABD0813421F481E0E0\n"
      ":107E6000C5D083E020C0823411F484E103C085349E\n"
      ":107E700019F485E0BBD091C0853581F499D0082FE5\n"
      ":107E800010E096D090E0982F8827802B912B880FB8\n"
      ":107E9000991F90930102809300027EC0863529F4D9\n"
      ":107EA00084E0A4D080E07CD078C0843609F04EC055\n"
      ":107EB00087D0E0910002F091010280E7E030F807FE\n"
      ":107EC00018F483E087BFE895C0E0D1E071D08993D2\n"
      ":107ED000809102028150809302028823B9F7E091D9\n"
      ":107EE0000002F091010280E7E030F80718F083E02B\n"
      ":107EF00087BFE89575D007B600FCFDCF4091000222\n"
      ":107F000050910102A0E0B1E02C9130E011968C91EB\n"
      ":107F1000119790E0982F8827822B932B1296FA01C5\n"
      ":107F20000C01D7BEE89511244E5F5F4FF1E0A038F9\n"
      ":107F3000BF0751F7E0910002F0910102E7BEE8951A\n"
      ":107F400007B600FCFDCFF7BEE89527C08437B9F42B\n"
      ":107F500037D046D0E0910002F09101023196F093C3\n"
      ":107F60000102E09300023197E4918E2F19D08091A5\n"
      ":107F70000202815080930202882361F70EC0853788\n"
      ":107F800039F42ED08EE10CD085E90AD08FE08BCF6A\n"
      ":107F9000813511F488E019D023D080E101D05CCF85\n"
      ":107FA000982F8091C00085FFFCCF9093C600089564\n"
      ":107FB000A8958091C00087FFFCCF8091C6000895EE\n"
      ":107FC000F7DFF6DF80930202F3CFE0E6F0E098E11E\n"
      ":107FD00090838083089580E0F8DFEE27FF270994DF\n"
      ":107FE000E7DF803209F0F7DF84E1DACF1F93182F43\n"
      ":0C7FF000DFDF1150E9F7F4DF1F91089566\n"
      ":0400000300007E007B\n"
      ":00000001FF\n"
    }
};

