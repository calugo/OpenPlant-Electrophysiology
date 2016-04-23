#include <EngduinoLEDs.h>
#include <EngduinoButton.h>

// Example that shows how to make a flashing red light like a
// rear cycle light. 
//
// Pushing the button changes modes - there are two types of
// flash, a constant mode and off.
//

// Defines the modes, as discussed above  
enum state_t {
  DARK,     // Off
  FLASH1,   // Regular flash
  FLASH2,   // Double flash 
  CONSTANT  // Constant on
};

// State
//
state_t ledState = FLASH1;  // the current state of the LEDs

void setup()
{
  EngduinoLEDs.begin();
  EngduinoButton.begin();
}

void loop() {
  // If the button was pressed since we last looked, change mode
  //
  if (EngduinoButton.wasPressed()) {
    switch (ledState) {
      case DARK:
        ledState = FLASH1;
        break;
      case FLASH1:
        ledState = FLASH2;
        break;
      case FLASH2:
        ledState = CONSTANT;
        break;
      case CONSTANT:
        ledState = DARK;
        break;
    }
  }
  
  // And flash the LEDs in an appropriate way
  // given the current mode
  //
  switch (ledState) {
    case DARK:
      EngduinoLEDs.setAll(OFF);
      break;
      
    case FLASH1:
      EngduinoLEDs.setAll(RED);
      delay(150);
      EngduinoLEDs.setAll(OFF);
      delay(150);
      break;
      
    case FLASH2:
      EngduinoLEDs.setAll(RED);
      delay(30);
      EngduinoLEDs.setAll(OFF);
      delay(70);
      EngduinoLEDs.setAll(RED);
      delay(30);
      EngduinoLEDs.setAll(OFF);
      delay(300);
      break;
      
    case CONSTANT:
      EngduinoLEDs.setAll(RED);
      break;
  }
}
