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
// ATTiny85 will require pull up resistors,
// but said resistors will cause hang after
// programming. Device needs to be reset
// or cycled after programming to run.
#include <Wire.h>
#include <avr/wdt.h>
#include <SendOnlySoftwareSerial.h>

/* Globals - well "ours" anyway */
SendOnlySoftwareSerial serLCD(1);  // Tx pin
unsigned short line1     = 0;
unsigned short line2     = 1;
unsigned int   baud      = 9600;
unsigned short intensity = 63;
unsigned short loopcount = 0;

void setup() {
  wdt_disable();
  serLCD.begin(baud);
  brightness_LCD(intensity);
  Wire.begin();
  delay(1000);
  preamble();
}

void loop() {

  byte error, address;
  int nDevices;
  char all[17] = "";
  char buff[3] = "";

  loopcount++;

  if (loopcount > 2) {
    loopcount = 0;
    reboot();
  }

  clear_LCD();
  beginLCDWrite(line1, 0);
  serLCD.print(F("Scanning...   "));
  endLCDWrite();

  nDevices = 0;

  for (address = 0; address < 128; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {

      // knock was answered

      if (nDevices) {
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

    else if (error == 4) {

      // not good

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

// Supposedly the definitive way to reboot an ATTiny85
// cleaner than SP=RAMEND;SREG=0;MCUSR=0;asm("rjmp 0")
// Not really essential, but revisits startup routines
// and causes initial information to redisplay.
void reboot(void) {
  cli();                        // suppress interrupts when touching WDIF  
  WDTCR = 0b11011000 | WDTO_1S; // refer "Watchdog Timer Control Register"
  sei();                        // interrupts back on 
  wdt_reset();                  // can't hurt ?   
  while (true) {}               // trap the sparks and wait.
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
// Also not that unusual for serial communication to have
// have beginTransmission..endTransmission.
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
