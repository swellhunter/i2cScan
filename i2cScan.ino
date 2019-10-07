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
SendOnlySoftwareSerial mySerial (1);  // Tx pin

void setup() {
  mySerial.begin(9600);
  brightness_lcd(63);
  Wire.begin();
  delay(1000);
  preamble();
}

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
SendOnlySoftwareSerial mySerial (1);  // Tx pin

void setup() {
  mySerial.begin(9600);
  brightness_lcd(63);
  Wire.begin();
  delay(1000);
  preamble();
}

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

  for (address = 0; address < 128; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered

      if (nDevices) strcat(all, ",");
      strcat(all, "0x");
      sprintf(buff, "%02X", address);
      strcat(all, buff);

      beginLCDWrite(1, 0);
      mySerial.print(F("Found 0x"));

      if (address < 16) {
        mySerial.print(F("0"));
      }

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

    if (nDevices > 1) {
      mySerial.print(F("s"));
    }

    mySerial.print(F("."));
  }

  endLCDWrite();
  delay(8000);
  clear_lcd();
  delay(200);
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

void brightness_lcd(unsigned short x) {
  mySerial.write(0xAA);
  mySerial.write(0x13);
  mySerial.write(x);
}
  for (address = 0; address < 128; address++) {

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered

      if (nDevices) strcat(all, ",");
      strcat(all, "0x");
      sprintf(buff, "%02X", address);
      strcat(all, buff);

      beginLCDWrite(1, 0);
      mySerial.print(F("Found 0x"));

      if (address < 16) {
        mySerial.print(F("0"));
      }

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

    if (nDevices > 1) {
      mySerial.print(F("s"));
    }

    mySerial.print(F("."));
  }

  endLCDWrite();
  delay(8000);
  clear_lcd();
  delay(200);
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

void brightness_lcd(unsigned short x) {
  mySerial.write(0xAA);
  mySerial.write(0x13);
  mySerial.write(x);
}
