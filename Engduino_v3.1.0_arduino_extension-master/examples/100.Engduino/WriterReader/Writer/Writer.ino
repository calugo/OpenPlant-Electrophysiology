#include <EngduinoLight.h>
#include <Wire.h>

// WriterReader - Writer example
//
// Demonstrates use of the Wire (I2C/TWI) library to establish
// wired communication between two or more Engduinos.
// In this example the Engduino is configured as Writer 
// (master device). It sends data to Slave device. 
// Data contains HI and LO byte value of the light sensor. 
// You can see, how change on this sensor affects slave 
// device by turning on and off different number of LEDs.
// For an example try to place boards closer together,
// facing to each other.
//
// CONNECTION:
// You need to use three wires to connect boards together
// IC2_SDA, ISC_SCL, GND (one-to-one)
// Help link: http://arduino.cc/en/Tutorial/MasterWriter
// 
// Illustrates use of Wire communication between two or
// more Engduino boards
//

// Address of board to communicate with.
#define READER_ADDRESS 10

void setup()
{
  // Join i2c bus (no need to define an address).
  Wire.begin(); 
  EngduinoLight.begin();
}

void loop() 
{  
  unsigned long lVal;
  // Read the light level.
  // The result is a 10 bit value - so in a range 0-1023.
  lVal = EngduinoLight.lightLevel();
  
  // Transmit to device #10.
  Wire.beginTransmission(READER_ADDRESS);
  
  Wire.write(highByte(lVal));  // Sends MSB of light level.     
  Wire.write(lowByte(lVal));   // Sends LSB of light level.       
  
  // Stop transmitting. 
  Wire.endTransmission();     

  delay(100); // Delay for 100 miliseconds.
}

