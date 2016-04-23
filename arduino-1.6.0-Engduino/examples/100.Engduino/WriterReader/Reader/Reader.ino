#include <EngduinoLEDs.h>
#include <Wire.h>

// WriterReader - Reader example
//
// Demonstrates use of the Wire (I2C/TWI) library to establish
// wired communication between two or more Engduinos.
// In this example the Engduino is configured as Reader 
// (slave device). It receives data from Master device. 
// Data contains HI and LO byte value from Master's light sensor. 
// You can see, how change on the Master's light sensor 
// affects this device by turning on and off different number 
// of LEDs. For an example try to place boards closer together,
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

// Address of this board.
#define MY_ADDRESS 10

int lightVal = 0;

void setup() 
{
  // Join i2c bus with address MY_ADDRESS.
  // Address must be the as Writer's sending address.
  Wire.begin(MY_ADDRESS); 
  // Register event on data recived.  
  Wire.onReceive(receiveEvent); 
  
  // Start serial for output
  Serial.begin(9600);           
  
  // Initialize RGB LEDs driver
  EngduinoLEDs.begin();
}

void loop()
{ 
  // The result is a 10 bit value - so in a range 0-1023
  // If we spread this between our 16 LEDs evenly, it means
  // we have to divide the value obtained by 64 to tell us
  // what the biggest numbered LED we have to light should be
  
  // In the reality we need to devide by 60 because devision
  // rounding.
  int lightTo = lightVal/60;
  
  for (int i = 0; i < lightTo; i++) {
    EngduinoLEDs.setLED(i, WHITE, 15);
  }
  for (int i = lightTo; i < 16; i++) {
    EngduinoLEDs.setLED(i, OFF);
  }
}

// Function that executes whenever data is received from master.
// This function is registered as an event, see setup().
void receiveEvent(int howMany)
{
  // We are expecting two bytes.
  if(howMany != 2) return;

  byte hi = Wire.read();  // Read MSB of light level.
  byte lo = Wire.read();  // Read LSB of light level.
  int lVal = word(hi,lo); // Combine them in one number.
    
  // Apply lowpass filter.
  lightVal = 0.5*lightVal + 0.5*lVal;
  // Print the received value of light level.
  Serial.print("Light level is: ");
  Serial.print(lVal); 
  Serial.print("   avg: ");
  Serial.println(lightVal);
}
