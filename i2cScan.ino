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
#include <Wire.h>
#include <SendOnlySoftwareSerial.h>

/* Globals - well "ours" anyway */
SendOnlySoftwareSerial serLCD(1);  // Tx pin

void setup() {
  serLCD.begin(9600);
  brightness_LCD(63);
  Wire.begin();
  delay(1000);
  preamble();
}

void loop() {

  byte error, address;
  int nDevices;
  char all[17] = "";
  char buff[3] = "";

  clear_LCD();
  beginLCDWrite(0, 0);
  serLCD.print(F("Scanning...   "));
  endLCDWrite();

  nDevices = 0;

  for (address = 0; address < 128; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {

      // knock was answered

      if (nDevices) strcat(all, ",");
      strcat(all, "0x");
      sprintf(buff, "%02X", address);
      strcat(all, buff);

      beginLCDWrite(1, 0);
      serLCD.print(F("Found 0x"));

      if (address < 16) {
        serLCD.print(F("0"));
      }

      serLCD.print(address, HEX);
      endLCDWrite();
      nDevices++;
      delay(2000);
      clear_LCD_line2();
    }

    else if (error == 4) {

      // not good

      beginLCDWrite(1, 0);
      serLCD.print(F("Error at : 0x"));
      if (address < 16){
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
    beginLCDWrite(0, 0);
    serLCD.print(F("No I2C devices!!"));
  }
  else {
    delay(1000);
    clear_LCD();

    beginLCDWrite(0, 0);
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

  // this endserLCDWrite() is for both branches above.
  // it may need to be rescoped if more elaborate
  // results blurb logic is required.
  endLCDWrite();
  delay(8000);
  clear_LCD();
  delay(200);
}

// Things in setup() best abstracted to declutter the
// code there and make it more readable.
void preamble(void) {
  clear_LCD();
  beginLCDWrite(0, 0);
  serLCD.print(F("*** I2C Scan ***"));
  endLCDWrite();
  delay(500);
  beginLCDWrite(1, 0);
  serLCD.print(F("... stand by ..."));
  endLCDWrite();
  delay(1000);
}

// serLCD functions, specific to RayLid, these are
// few enough to not need a library or separate
// file.

// This sets the brightness of the serLCD. Note that the
// relationship is not that linear. You should test
// for fixed values on the scale that are useful.
void brightness_LCD(unsigned short x) {
  serLCD.write(0xAA);
  serLCD.write(0x13);
  serLCD.write(x);
}

// This clears the whole serLCD display.
void clear_LCD(void) {
  serLCD.write(0xAA);
  serLCD.write(0x10);
}

// This is the preamble for the UART display to specify
// that characters for writing to a particular location
// are to follow. Note order of r,c as may not follow
// usual convention for serLCD displays? (reversed?).
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
// UART serLCD. There should not have been any \n in that
// stream either.
void endLCDWrite(void) {
  serLCD.write(0x0D);
}

// serLCD displays perform at their best when blanked
// between "shifting in" writes of large clumps of
// text. This clears the 2nd of 2 lines.
void clear_LCD_line2(void) {

  unsigned short i;

  beginLCDWrite(1, 0);
  for (i = 0; i < 16; i++) {
    serLCD.print(F(" "));
  }
  endLCDWrite();
  delay(500);
}
