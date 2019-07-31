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

//#include <SendOnlySoftwareSerial.h>
//SendOnlySoftwareSerial serial (1);  // Tx pin

//#include <SoftwareSerial.h>
//SoftwareSerial serial(7, 1); // RX (out of range), TX

/*
   This is the standard "run once" routine.
*/
void setup() {
  Serial.begin(9600);
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

  clear_lcd();
  beginLCDWrite(0, 0);
  Serial.print(F("Scanning..."));
  endLCDWrite();

  nDevices = 0;

  for (address = 1; address < 128; address++) {
    char adrs[4];
    itoa((int)address,adrs,10);
    beginLCDWrite(0, 13);
    Serial.print(adrs);
    endLCDWrite();
    delay(100);
        
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered
      beginLCDWrite(1, 0);
      Serial.print(F("Found: 0x"));
      if (address < 16) Serial.print(F("0"));
      Serial.print(address, HEX);
      endLCDWrite();
      nDevices++;
      delay(4000);
    }
    else if (error == 4) {
      beginLCDWrite(1, 0);
      Serial.print(F("Error at : 0x"));
      if (address < 16) Serial.print(F("0"));
      Serial.print(address, HEX);
      endLCDWrite();
      delay(4000);
    }

  }

  if (nDevices == 0) {
    clear_lcd();
    beginLCDWrite(0, 0);
    Serial.print(F("No I2C devices!!"));
  }
  else {
    delay(4000);
    clear_lcd();
    beginLCDWrite(1, 0);
    Serial.print(F("Scan done."));
  }

  endLCDWrite();
  delay(4000);
  asm("jmp 0");
}

void clear_lcd(void) {
  Serial.write(0xAA);
  Serial.write(0x10);
}


void beginLCDWrite(unsigned char r, unsigned c) {
  Serial.write(0xAA);
  Serial.write(0x20);
  Serial.write(r);
  Serial.write(c);
  Serial.write(0xAA);
  Serial.write(0x25);
}

/*
   So the million dollar question is can
   this just be replaced with \n?
   Best to keep it in case of differing
   hardware.
*/
void endLCDWrite(void) {
  Serial.write(0x0D);
}

/*
   Things in setup() best abstracted
*/
void preamble(void) {
  Serial.write(0xAA);               // Hardware info.
  Serial.write((uint8_t)0x00);
  delay(2000);
  clear_lcd();
  beginLCDWrite(0, 0);
  Serial.print(F("*** I2C Scan ***"));
  endLCDWrite();
  delay(500);
  beginLCDWrite(1, 0);
  Serial.print(F("... stand by ..."));
  endLCDWrite();
  delay(1000);
}

void brightness(unsigned short x) {
  Serial.write(0xAA);
  Serial.write(0x13);
  Serial.write(x);
}
