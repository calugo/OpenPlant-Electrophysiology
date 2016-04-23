#include <EngduinoLEDs.h>

/*
 * Example of a manual button debounce. The code for this has been
 * slightly adapted for the Engduino from 
 * http://www.arduino.cc/en/Tutorial/Debounce
 *
 * It is not necessary manually to debounce the button on the Engduino.
 * The same effect can be achieved by using the EngduinoButton library
 * which hides the details. However, debouncing is a perhaps unexpected
 * essential in systems with buttons so this provides a good example
 * of how to deal with an issue that occurs in the real world.
 *
 */

// State
//
int buttonState     = HIGH;  // the current reading from the input pin
int lastButtonState = HIGH;  // the previous reading from the input pin
int ledState        = HIGH;  // the current state of the output pin

// the following variables are longs because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;   // the last time the output pin was toggled
long debounceDelay    = 50;  // the debounce time; increase if the output flickers

void setup()
{
  EngduinoLEDs.begin();
  pinMode(BUTTON, INPUT);
  EngduinoLEDs.setAll(ledState == HIGH ? RED : OFF);
}

void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTON);

  // check to see if you just pressed the button
  // (i.e. the input went from HIGH to LOW),  and you've waited
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is LOW
      if (buttonState == LOW) {
        ledState = !ledState;
      }
    }
  }
 
  // set the LEDs
  EngduinoLEDs.setAll(ledState == HIGH ? RED : OFF);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}
