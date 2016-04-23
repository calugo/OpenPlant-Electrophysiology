#include <EngduinoLEDs.h>;
#include <EngduinoThermistor.h>;

// This is a very simple program to demonstrate that it is
// possible to communicate with the Engduino over the serial
// connection - both from and to the Engduino. One example use
// for this is to communicate with a program written in Python
// on a Raspberry Pi - the bulk of the code can be written on the
// Pi and the Engduino used for illuminating LEDs or as a sensor
// input to the Pi.
//
char val = ' ';

void setup() {
  Serial.begin(115200);
  EngduinoLEDs.begin();
  EngduinoThermistor.begin();
}

void loop() {
  // If the serial is not available, loop until it is.
  //
  if (Serial.available()) {
    // The serial is available, so wait for the next character
    // to be sent on it.
    //
    val = Serial.read();
    
    // If the character is:
    //  'r' switch the LEDs to red
    //  'g' switch the LEDs to green
    //  'b' switch the LEDs to blue
    //  'o' switch the LEDs off
    //  't' measure temperature and return this value
    //
    if (val == 'r') {
      EngduinoLEDs.setAll(RED);
    } 
    else if (val == 'g') {
      EngduinoLEDs.setAll(GREEN);
    } 
    else if (val == 'b') {
      EngduinoLEDs.setAll(BLUE);
    } 
    else if (val == 'o') {
      EngduinoLEDs.setAll(OFF);
    } 
    else if (val == 't') {
      // Get the temperature
      float t = EngduinoThermistor.temperature();

      // And return the value by writing to the serial port.
      Serial.print(t);
    }
  }
}

