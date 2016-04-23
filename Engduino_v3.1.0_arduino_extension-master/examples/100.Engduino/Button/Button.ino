#include <EngduinoLEDs.h>
#include <EngduinoButton.h>

// Simple test of a button
// Push the button to toggle the LEDs

bool ledsOn = false;    // State of the LEDs - initially off.

void setup()
{
  EngduinoLEDs.begin();
  EngduinoButton.begin();
}

void loop() {
  
  // Wait until the button has been pressed.
  //
  EngduinoButton.waitUntilPressed();

  // If the LEDs were on, turn them off and vice versa
  //
  if (ledsOn) {
    ledsOn = false;
    EngduinoLEDs.setAll(OFF);
  }
  else {
    ledsOn = true;
    EngduinoLEDs.setAll(RED);    
  }
  
}
