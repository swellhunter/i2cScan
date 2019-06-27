/****************************************************
  i2c_Scan - Ripped off from Arduino Playground
  Adapted for ATTiny45/85 with UART LCD or terminal.
  ___    +---v---+
  RST   1|o      |8  VCC
  XTAL  2|       |7  SCL  *
  XTAL  3|       |6  TX   *
  GND   4|       |5  SDA  *
         +-------+
  Requires Dr Azzy's ATTinyCore
  https://github.com/SpenceKonde/ATTinyCore
  add the following URL to boards manager:
  http://drazzy.com/package_drazzy.com_index.json

  Uses : nickgammon/SendOnlySoftwareSerial
*****************************************************/
#include <Wire.h>

#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial mySerial (1);  // Tx pin

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(7, 1); // RX (out of range), TX

/********************************************
   Needs work for Serial Control Characters
   if to be used with UART LCD.
 ********************************************/
void setup() {
  Wire.begin();
  mySerial.begin(9600);
  mySerial.println(F("\nI2C Scanner"));
}

void loop() {
  byte error, address;
  int nDevices;

  mySerial.println(F("Scanning..."));

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // knock was answered
      mySerial.print(F("I2C device found at address 0x"));
      if (address < 16) mySerial.print(F("0"));
      mySerial.print(address, HEX);
      mySerial.println("  !");
      nDevices++;
    }
    else if (error == 4) {
      mySerial.print(F("Unknown error at address 0x"));
      if (address < 16)mySerial.print(F("0"));
      mySerial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    mySerial.println(F("No I2C devices found\n"));
  else
    mySerial.println(F("done\n"));

  delay(10000); // Should poweroff or sleep?
}
