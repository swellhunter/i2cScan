/****************************************************
  https://github.com/swellhunter/i2cScan
  i2cScanS - Ripped off from Arduino Playground
  Adapted for ATTiny45/85 with UART LCD or terminal.
  This version specifically for RSI1602***-00
  See https://share.weiyun.com/5if7UNG
   
  ___    +---v---+
  RST   1|o      |8  VCC
  XTAL  2|       |7  SCL  *
  XTAL  3|       |6  TX   *
  GND   4|       |5  SDA  *
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
//SoftwareSerial mySerial(7, 1); // RX (out of range), TX

/*
   This is the standard "run once" routine.
*/
void setup() {
  Wire.begin();
  mySerial.begin(9600);
  delay(1000);
  preamble();
}

/*
   This is the endless loop body
*/

void loop() {

  byte error, address;
  int nDevices;

  clear_lcd();
  beginLCDWrite(0, 0);
  mySerial.print(F("Scanning..."));
  endLCDWrite();

  nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered
      beginLCDWrite(1, 0);
      mySerial.print(F("Found at : 0x"));
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

  clear_lcd();

  if (nDevices == 0) {
    beginLCDWrite(0, 0);
    mySerial.print(F("No I2C devices!!"));
  }
  else {
    mySerial.print(F("Scan done."));
  }
  
  endLCDWrite();
  delay(4000);
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
 * Things in setup() best abstracted
 */
void preamble(void) {
  mySerial.write(0xAA);               // Hardware info.
  mySerial.write((uint8_t)0x00);
  delay(1000);
  clear_lcd();
  beginLCDWrite(0, 0);
  mySerial.print(F("*** I2C Scan ***"));
  endLCDWrite();
  delay(500);
  beginLCDWrite(1, 0);
  mySerial.print(F("... stand by ..."));
  endLCDWrite();
  delay(500);
}
