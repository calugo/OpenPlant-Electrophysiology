/**
* \defgroup EngduinoButton Driver for Engduino Button
* 
* @{
*/

/**
* \file 
* 		Engduino Button driver
* \author
* 		Engduino team: support@engduino.org
*/

#ifndef __ENGDUINOBUTTON_H__
#define __ENGDUINOBUTTON_H__

#include <Arduino.h>
#include <Engduino.h>

#define DEBOUNCE_DELAY 50

// Needed to ensure correct linkage between C++ and C linkage of ISR
extern "C" void INT6_vect(void) __attribute__ ((signal));

class EngduinoButtonClass 
{
	private:
		/*
		 * Timings on this need to be stored as longs, simply because millis(),
		 * which we use for timing, returns a long and we would otherwise encounter
		 * problems with wrapping
		 */
		volatile long lastChangeTime;   // The last time the button input changed
		volatile bool buttonState; 		// Whether the button is currently pressed
		volatile bool fallingEdge;		// Whether we're looking for a falling or rising edge in the ISR
		volatile bool wasPressedState;	// Whether the button has been pressed since we last checked
		volatile bool wasReleasedState;	// Whether the button has been released since we last checked
		long debounceDelayTime;			// How long we will wait until saying the switch is settled

	public:
		EngduinoButtonClass();
		void begin(long debounceDelay=DEBOUNCE_DELAY);
		void end();
		void reset();

		bool isPressed();
		void waitUntilPressed();
		void waitUntilReleased();
		bool wasPressed();
		bool wasReleased();

		/*
		 * The ISR needs access to the private variables, so we declare it
		 * a friend of the class
		 */
		friend void INT6_vect();
};

extern EngduinoButtonClass EngduinoButton;

#endif

/** @} */
