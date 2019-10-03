/****************************************************
  https://github.com/swellhunter/i2cScan
  i2cScan - Ripped off from Arduino Playground
  Adapted for ATTiny85 with UART LCD or terminal.
  ___    +---v---+
  RST   1|o      |8  VCC
  XTAL  2|       |7  SCL  * two wire serial clock
  XTAL  3|       |6  TX   * UART serial out
  GND   4|       |5  SDA  * two wire serial data
         +-------+
  Requires :
  https://github.com/nickgammon/SendOnlySoftwareSerial
  https://github.com/SpenceKonde/ATTinyCore
  add the following URL to boards manager:
  http://drazzy.com/package_drazzy.com_index.json
******************************************************/
#include <Wire.h>
#include <avr/wdt.h>
#include <SendOnlySoftwareSerial.h>

SendOnlySoftwareSerial mySerial (1);  // Tx pin

// This is the standard "run once" routine.
void setup() {
  // https://bigdanzblog.wordpress.com/2015/07/20/resetting-rebooting-attiny85-with-watchdog-timer-wdt/
  wdt_disable();
  mySerial.begin(9600);
  brightness(63);
  Wire.begin();
  delay(1000);

  clear_lcd();
  beginLCDWrite(0, 0);
  mySerial.print(F("RAMEND : 0x0"));
  mySerial.print(RAMEND, HEX);
  endLCDWrite();
  beginLCDWrite(1, 0);
  mySerial.print(F("Stack  : 0x0"));
  mySerial.print(SP, HEX);
  endLCDWrite();

  delay(8000);
  preamble();

}

// This is the endless loop body
void loop() {

  byte error, address;
  int nDevices;
  char all[17] = "";
  char buff[3] = "";

  clear_lcd();
  beginLCDWrite(0, 0);
  mySerial.print(F("Scanning...   "));
  endLCDWrite();

  nDevices = 0;

  for (address = 1; address < 128; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered

      if (nDevices) strcat(all, ",");
      strcat(all, "0x");
      sprintf(buff, "%02X", address);
      strcat(all, buff);

      beginLCDWrite(1, 0);
      mySerial.print(F("Found 0x"));
      if (address < 16) mySerial.print(F("0"));
      mySerial.print(address, HEX);
      endLCDWrite();
      nDevices++;
      delay(2000);
      clear_lcd_line2();
    }
    else if (error == 4) {
      beginLCDWrite(1, 0);
      mySerial.print(F("Error at : 0x"));
      if (address < 16) mySerial.print(F("0"));
      mySerial.print(address, HEX);
      endLCDWrite();
      delay(4000);
      clear_lcd_line2();
    }
  }

  if (nDevices == 0) {
    clear_lcd();
    beginLCDWrite(0, 0);
    mySerial.print(F("No I2C devices!!"));
  }
  else {
    delay(1000);
    clear_lcd();

    beginLCDWrite(0, 0);
    mySerial.print(all);
    endLCDWrite();

    beginLCDWrite(1, 0);
    mySerial.print(F("Done. "));
    mySerial.print(nDevices);
    mySerial.print(F(" hit"));
    if (nDevices > 1) mySerial.print(F("s"));
    mySerial.print(F("."));
  }

  endLCDWrite();
  delay(8000);
  reminder();
  delay(4000);
  reboot2();

}

void clear_lcd(void) {
  mySerial.write(0xAA);
  mySerial.write(0x10);
}

void clear_lcd_line2(void) {

  unsigned short i;

  beginLCDWrite(1, 0);
  for (i = 0; i < 16; i++) {
    mySerial.print(F(" "));
  }
  endLCDWrite();
  delay(500);
}

void beginLCDWrite(unsigned char r, unsigned c) {
  mySerial.write(0xAA);
  mySerial.write(0x20);
  mySerial.write(r);
  mySerial.write(c);
  mySerial.write(0xAA);
  mySerial.write(0x25);
}

/*
   So the million dollar question is can
   this just be replaced with \n?
   Best to keep it in case of differing
   hardware.
*/
void endLCDWrite(void) {
  mySerial.write(0x0D);
}

/*
   Things in setup() best abstracted
*/
void preamble(void) {
  mySerial.write(0xAA);               // Hardware info.
  mySerial.write((uint8_t)0x00);
  delay(2000);
  clear_lcd();
  beginLCDWrite(0, 0);
  mySerial.print(F("*** I2C Scan ***"));
  endLCDWrite();
  delay(500);
  beginLCDWrite(1, 0);
  mySerial.print(F("... stand by ..."));
  endLCDWrite();
  delay(1000);
}

void brightness(unsigned short x) {
  mySerial.write(0xAA);
  mySerial.write(0x13);
  mySerial.write(x);
}

//void reBootTiny85() {
// Quick, dirty, effective....
// This is why people prefer WDT ?
//  SREG  = 0;
//  MCUSR = 0;
//  SP    = RAMEND;
//  asm("rjmp 0");
//}

void reminder(void) {
  clear_lcd();
  delay(250);
  beginLCDWrite(0, 0);
  mySerial.print(F("** Please -   **"));
  endLCDWrite();
  beginLCDWrite(1, 0);
  mySerial.print(F("** switch off **"));
  endLCDWrite();
}

void reboot2() {
#if defined (__AVR_ATtiny85__)
  cli();
  // WDIF = TRUE    refer "Watchdog Timer Control Register"
  // WDIE = TRUE
  // WDP3   not set
  // WDCE = TRUE
  // WDE  = TRUE
  // WDP2,WDP1,WDP0 = 110
  WDTCR = 0xD8 | WDTO_1S;
  sei();
  wdt_reset();
#else
  wdt_disable();
  wdt_enable(WDTO_15MS);
#endif
  // trap the sparks and wait.
  while (true) {}
}
