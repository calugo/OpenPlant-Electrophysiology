/**
* \defgroup EngduinoLEDs Driver for Engduino LEDS
* 
* @{
*/

/**
* \file 
* 		Engduino LED driver
* \author
* 		Engduino team: support@engduino.org
*/

#ifndef __ENGDUINOLEDS_H__
#define __ENGDUINOLEDS_H__

#include <Arduino.h>
#include <Engduino.h>

enum colour {
	RED,
	GREEN,
	BLUE,
	YELLOW,
	MAGENTA,
	CYAN,
	WHITE,
	OFF
};

#define MAX_BRIGHTNESS 0x0F

// Needed to ensure correct linkage between C++ and C linkage of ISR
extern "C" void TIMER4_COMPA_vect(void) __attribute__ ((signal));

class EngduinoLEDsClass 
{
	private:
		/* 
		 * The xSet variables contain the values actually set by the user. RGB
		 * values run from 0-16 in each channel 
		 */
		uint8_t RSet[16]; 
		uint8_t GSet[16];
		uint8_t BSet[16];
	
		/* 
		 * The xAccum and xDisp variables contain values used in the PWM
		 * implementation. The Accum variables are counter variables that
		 * increment at each time point by an amount equal to the brightness
		 * given to the channel (0-16). When the counter ticks over, the
		 * corresponding Disp bit is set, otherwise it is unset. Each
		 * tick, the calculated Disp values are sent to the LED drivers  
		 */
		volatile uint8_t RAccum[16];
		volatile uint8_t GAccum[16];
		volatile uint8_t BAccum[16];
	
		volatile uint8_t RDisp[16];
		volatile uint8_t GDisp[16];
		volatile uint8_t BDisp[16];
		
		void _setLED(uint8_t LEDidx, colour c, uint8_t brightness);
		void _setLED(uint8_t LEDidx, uint8_t r, uint8_t g, uint8_t b);
	
	public:
		EngduinoLEDsClass();
		void begin(); // Default
		void end();

		void setLED(uint8_t LEDNumber, colour c);
		void setLED(uint8_t LEDNumber, colour c, uint8_t brightness);
		void setLED(uint8_t LEDNumber, uint8_t r, uint8_t g, uint8_t b);

		void setAll(colour c);
		void setAll(colour c, uint8_t brightness);
		void setAll(uint8_t r, uint8_t g, uint8_t b);
				
		void setLEDs(colour c[16]);
		void setLEDs(colour colour[16], uint8_t brightness[16]);
		void setLEDs(uint8_t r[16], uint8_t g[16], uint8_t b[16]);
		void setLEDs(uint8_t rgb[3][16]);
		void setLEDs(uint8_t rgb[16][3]);

		/*
		 * The ISR needs access to the private variables, so we declare it
		 * a friend of the class
		 */
		friend void TIMER4_COMPA_vect();
};


extern EngduinoLEDsClass EngduinoLEDs;

#endif

/** @} */
