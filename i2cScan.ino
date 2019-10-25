/****************************************************
  https://github.com/swellhunter/i2cScan
  i2cScan - Ripped off from Arduino Playground
  Adapted for ATTiny85 with RayLid UART LCD.
  ___    +---v---+
  RST   1|o      |8  VCC
  XTAL  2|       |7  SCL  * two wire serial clock
  XTAL  3|       |6  TX   * UART serial out
  GND   4|       |5  SDA  * two wire serial data
         +-------+
  Requires :
  https://github.com/nickgammon/SendOnlySoftwareSerial
  https://github.com/SpenceKonde/ATTinyCore
  Raylid RSI1602***-00 16*02 UART LCD display.
  (from Ebay user surenoo)
  2 x 4.7KΩ pull up resistors for IIC lines.
  Physical switch for power on and off.
******************************************************/

/* Includes */
// ATTiny85 will require pull-up resistors,
// but said resistors will cause hang after
// programming. Device needs to be reset
// or cycled after programming to run.
#include <Wire.h>
#include <avr/wdt.h>

#ifdef wdt_enable
   #undef wdt_enable
#endif

// Alrighty then. ATTiny85 does not have a CCP register.
// There is no "good" wdt_enable() defined in avr/wdt.h 
// See WDTCR section in datasheet for the 'x' values.
// Still need to test for "one punch" effectiveness ala bigdanzblog. Datasheet concerns disable.
// https://bigdanzblog.wordpress.com/2015/07/20/resetting-rebooting-attiny85-with-watchdog-timer-wdt/
// 15 Years and nobody has fixed this.
// Of course it would be a lot simpler, but less portable to redefine WDTO_4S and WDTO_8S

//    in Rnn,SREG                          ; stash SREG
//    cli                                  ; disable interrupts
//    wdr                                  ; watchdog reset
// ;    WDIF  WDIE  WDP3  WDCE  WDE  WDP2  WDP1  WDP0    refer "Watchdog Timer
// ;      1     1     0     1     1    0     0     0     Control Register"
//    out WDTCR, 0b11011000                ; change enable "0xD8", redundant? 4 x _BV() maybe ?
//    out WDTCR, 0b11011000 | 0b00x00xxx   ; supply WDTO value but mind 8 and 9, see wdt.h, datasheet 
//    out SREG,  Rnn ; restore SREG        ; put SREG back

#define wdt_enable(value) \
__asm__ __volatile__ ( \
    "in __tmp_reg__,__SREG__" "\n\t"  \
    "cli" "\n\t"  \
    "wdr" "\n\t"  \
    "out %[WDTREG],%[SIGNATURE]" "\n\t"  \
    "out %[WDTREG],%[WDVALUE]" "\n\t"  \
    "out __SREG__,__tmp_reg__" "\n\t"  \
    : /* no outputs */  \
    : [SIGNATURE] "r" ((uint8_t)0xD8), \
      [WDTREG] "I" (_SFR_IO_ADDR(_WD_CONTROL_REG)), \
      [WDVALUE] "r" ((uint8_t)(0xD8 \
      | (value & 0x08 ? _WD_PS3_MASK : 0x00) \
      | _BV(WDE) | (value & 0x07) )) \
    : "r16" \
)

#include <SendOnlySoftwareSerial.h>

/* Globals - well "ours" anyway */
SendOnlySoftwareSerial serLCD(1);  // Tx pin
unsigned short line1     = 0;
unsigned short line2     = 1;
unsigned int   baud      = 9600;
unsigned short intensity = 63;
unsigned short loopcount = 0;

// This is the ubiquitous Arduino setup function
// It is called once per reboot.  
void setup() {
  wdt_reset();
  // Spence Konde recommends, could save first.
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
  serLCD.begin(baud);
  brightness_LCD(intensity);
  Wire.begin();
  delay(1000);
  preamble();
}

// And this is the main Arduino loop that is 
// executed indefinitely, the stack is already 
// down about 8 bytes when we get in here.
void loop() {

  byte error, address;
  int nDevices;
  char all[17] = ""; // one whole line, right ?
  char buff[3] = ""; // hold two hex characters.

  loopcount++;       // do it here, not miles away. 

  // In theory we have a spare PortB pin to run 
  // a piezo buzzer to remind people to switch 
  // off after 4 cycles, ala fridge door and 
  // then reset after a delay.  
  if (loopcount > 4) {
    reboot();
  }

  clear_LCD();
  beginLCDWrite(line1, 0);
  serLCD.print(F("Scanning...   "));
  endLCDWrite();

  nDevices = 0;

  // Most of this loop ripped off from Arduino Playground.
  for (address = 0; address < 128; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered

      if (nDevices) { // separate with commas
        strcat(all, ",");
      }
      else {
        strcat(all, "0x");
      }

      sprintf(buff, "%02X", address);
      strcat(all, buff);

      beginLCDWrite(line2, 0);
      serLCD.print(F("Found 0x"));

      if (address < 16) {
        serLCD.print(F("0"));
      }

      serLCD.print(address, HEX);
      endLCDWrite();
      nDevices++;
      delay(1500);
      clear_LCD_line2();
    }

    else if (error == 4) {   // not good
      beginLCDWrite(line2, 0);
      serLCD.print(F("Error at : 0x"));
      if (address < 16) {
        serLCD.print(F("0"));
      }
      serLCD.print(address, HEX);
      endLCDWrite();
      delay(4000);
      clear_LCD_line2();
    }
  }

  if (nDevices == 0) {
    clear_LCD();
    beginLCDWrite(line1, 0);
    serLCD.print(F("No I2C devices!!"));
  }
  else {
    delay(1000);
    clear_LCD();

    beginLCDWrite(line1, 0);
    serLCD.print(all);
    endLCDWrite();

    beginLCDWrite(1, 0);
    serLCD.print(F("Done. "));
    serLCD.print(nDevices);
    serLCD.print(F(" hit"));

    if (nDevices > 1) {
      serLCD.print(F("s"));
    }

    serLCD.print(F("."));
  }

  // this endLCDWrite() is for both branches above.
  // it may need to be rescoped if more elaborate
  // results blurb logic is required.
  endLCDWrite();
  delay(8000);
  clear_LCD();
  delay(200);
}

//---------------------------------------------------
// functions and subroutines follow...
//----------------------------------------------------

// Things in setup() best abstracted to declutter the
// code there and make it more readable.
void preamble(void) {
  id_LCD();
  clear_LCD();
  beginLCDWrite(line1, 0);
  serLCD.print(F("*** I2C Scan ***"));
  endLCDWrite();
  delay(1000);
  beginLCDWrite(line2, 0);
  serLCD.print(F("... stand by ..."));
  endLCDWrite();
  delay(2000);
}

// Supposedly the definitive way to reboot an ATTiny85.
// Cleaner than SP=RAMEND;SREG=0;MCUSR=0;asm("rjmp 0")
// Not really essential, but revisits startup routines
// and causes initial information to redisplay.
// Could also tie a spare output pin to reset....
// Also could just trap the sparks and have a button for the user?
void reboot(void) {
  cli();                        // suppress interrupts when touching WDIF
  // (WD)IF  IE  P3  CE  DE  P2  P1  P0            refer "Watchdog Timer
  //      1   1   0   1   1   0   0   0            Control Register"
  WDTCR = 0b11011000 | WDTO_1S; // (WDTO_1S = 6 = 110)
  // Now, we need to follow up if that WDTO value has "taken" and make
  // sure it is not just using the old one? Changes are supposed to be
  // a multi-step process? Further, after 15 years or more why does the 
  // stock wdt.h not work? Anybody?  The above approach from bigdanzblog.
  
  // Note also that values 8,9 for 4s and 8s will not work. This is stated
  // in wdt.h and is because P3 is an "island". See datasheet table.
  // They can only be used with wdt_enable() which is broken for the ATTiny85
  // anyway?
  
  sei();                        // interrupts back on
  wdt_reset();                  // not needed if we are forcing it?
  while (true) {}               // trap the PC (sparks?) and wait.
}

//---------------------------------------------------
// LCD functions, specific to RayLid, these are
// few enough to not need a library or separate
// file. Anyway we are using our instance explicitly.
//----------------------------------------------------

// This sets the brightness of the LCD. Note that the
// relationship is not that linear. You should test
// for fixed values on the scale that are useful.
void brightness_LCD(unsigned short x) {
  serLCD.write(0xAA);
  serLCD.write(0x13);
  serLCD.write(x);
}

// This clears the whole LCD display.
void clear_LCD(void) {
  serLCD.write(0xAA);
  serLCD.write(0x10);
}

// These are the control characters for the UART display
// to specify that characters for writing to a particular
// location are to follow. Note order of r,c as may not
// follow the usual convention for LCD displays? (reversed?).
// Also it is not that unusual for serial communication to 
// have beginTransmission..endTransmission anyway?
void beginLCDWrite(uint8_t r, uint8_t c) {
  serLCD.write(0xAA);
  serLCD.write(0x20);
  serLCD.write(r);
  serLCD.write(c);
  serLCD.write(0xAA);
  serLCD.write(0x25);
}

// This terminates the character stream written to the
// UART LCD. There should not have been any \n in that
// stream either.
void endLCDWrite(void) {
  serLCD.write(0x0D);
}

// LCD displays perform at their best when blanked
// between "shifting in" writes of large clumps of
// text. This clears the 2nd of 2 lines.
void clear_LCD_line2(void) {

  unsigned short i;

  beginLCDWrite(line2, 0);

  for (i = 0; i < 16; i++) {
    serLCD.print(F(" "));
  }

  endLCDWrite();
  delay(500);
}

// Dump model and version (if any) of the LCD
// display. Obviously we can see the colour,
// and we know it is UART. The "K" means the 
// keyboard/pad inputs will work (wasted here).
void id_LCD(void) {
  serLCD.write(0xAA);
  serLCD.write((uint8_t)0);  // note the "cast".
  delay(2000);
}
