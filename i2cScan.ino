/****************************************************
  https://github.com/swellhunter/i2cScan
  i2cScan - Ripped off from Arduino Playground
  Adapted for ATTiny45/85 with UART LCD or terminal.
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

#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial mySerial (1);  // Tx pin

//#include <SoftwareSerial.h>
//SoftwareSerial serial(7, 1); // RX (out of range), TX

/*
   This is the standard "run once" routine.
*/
void setup() {
  mySerial.begin(9600);
  brightness(63);
  Wire.begin();
  delay(1000);
  preamble();
}

/*
   This is the endless loop body
*/

void loop() {

  byte error, address;
  int nDevices;
  char all[17] = "";
  char buff[3] = "";

  clear_lcd();
  beginLCDWrite(0, 0);
  //mySerial.print(F("Scanning... 0x"));
  mySerial.print(F("Scanning...   "));
  endLCDWrite();

  nDevices = 0;

  for (address = 1; address < 128; address++) {

    //   beginLCDWrite(0, 14);
    //   if (address < 16){
    //     mySerial.print(F("0"));
    //   }
    //   mySerial.print(address, HEX);
    //   endLCDWrite();
    //   delay(100);

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
    }
    else if (error == 4) {
      beginLCDWrite(1, 0);
      mySerial.print(F("Error at : 0x"));
      if (address < 16) mySerial.print(F("0"));
      mySerial.print(address, HEX);
      endLCDWrite();
      delay(4000);
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
  delay(4000);
  asm("rjmp 0");
}

void clear_lcd(void) {
  mySerial.write(0xAA);
  mySerial.write(0x10);
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
