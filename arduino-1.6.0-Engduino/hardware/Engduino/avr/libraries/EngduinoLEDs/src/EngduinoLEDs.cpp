/**
* \addtogroup EngduinoLEDs
*
* This is the driver code for LEDs on the Engduino
* These LEDS are not directly connected to pins on the
* AtMega32u4 processor. Instead they are connected through
* LED drivers so should only be accessed through this code.
*
* The Engduino has 16 RGB LEDs on it, each of which
* can be controlled independently. To make the granularity
* of control greater, we implement a software PWM for each
* LED. This allows for 16 levels of brightness in each of
* the RGB channels on each LED with minimal flicker.
* 
* This implementation uses TIMER4 Comparator A to
* implement the PWM. The timer is set to run at ~320Hz. The
* clock is reset on interrupt, so you should be very wary
* about using the other comparators.
*
* @{
*/

/**
* \file 
* 		Engduino LED driver
* \author
* 		Engduino team: support@engduino.org
*/

#include "pins_arduino.h"
#ifdef __BOARD_ENGDUINOV3
	#include <SPI.h>
#endif
#include "EngduinoLEDs.h"

/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
* 
* C++ constructor for this class. Empty.
*/
EngduinoLEDsClass::EngduinoLEDsClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
*
* The connection to the LEDs is not direct. Instead, it happens through
* three LED drivers - one each for the R, G, and B channels. These are
* connected through a daisy-chained SPI connection. There is a single latch
* line connected to all driver chips, which latches the value set by SPI when
* the line is pulsed high. The output from each driver chip can be switched on
* or off with a separate output enable line to each driver - when these are LOW
* the driver output lines go to the LEDs; when HIGH, they do not.
* 
*/
void EngduinoLEDsClass::begin() 
{
	for (int i = 0; i < 15; i++) {
	  RSet[i]   = GSet[i]   = BSet[i]   = 0;
	  RAccum[i] = GAccum[i] = BAccum[i] = 0;
	  RDisp[i]  = GDisp[i]  = BDisp[i]  = 0;
	}
	
	// RGB LED drivers - output enable lines
	pinMode(LED_R_OE, OUTPUT);
	pinMode(LED_G_OE, OUTPUT);
	pinMode(LED_B_OE, OUTPUT);

	// Disable the output from the LED drivers while we set up
	digitalWrite(LED_R_OE, HIGH);
	digitalWrite(LED_G_OE, HIGH);
	digitalWrite(LED_B_OE, HIGH);

#if defined(__BOARD_ENGDUINOV1)||defined(__BOARD_ENGDUINOV2)
	pinMode(LED_MISO,  INPUT);
	pinMode(LED_MOSI,  OUTPUT);
	pinMode(LED_SCLK,  OUTPUT);
	pinMode(LED_LATCH, OUTPUT);
	// Clock line is initially low.
	// We must pulse high to tick
	digitalWrite(LED_SCLK, LOW);
#elif defined (__BOARD_ENGDUINOV3)	
	// Set up SPI data connection to the RGB LEDs
	// start the SPI library:
    SPI.begin();
	pinMode(LED_LATCH, OUTPUT);
#endif
	
	
	// Latch line is initially low
	// We must pulse high to latch
	digitalWrite(LED_LATCH, LOW);
	
	// The buffers might have something latched from the
	// last time they were programmed, so remove it - nothing
	// will be shown, because the LED driver OE lines are pulled
	// high at this point in time.
	setAll(OFF);
	
	// Set up timer 4. We use a software PWM to drive the LEDs to control both
	// brightness and colour. This requires a tick, which we set at 320Hz - this
	// is determined by the fact that we have 16 brightness levels per channel on
	// our PWM. Brightness 1 corresponds to 1 on period and 15 off periods. To
	// avoid flicker, we need this to happen at >15-18Hz, so we choose to run
	// through the 16 on/off periods 20 times per second, meaning that we need
	// a timer tick of 320Hz.
	//
	// Timer 4 is a free running 8 or 10 bit timer; NOTE: there is no CTC
	// (Clear Timer on Compare match) mode as for timers 1 and 3, so we must
	// manually reset the clock when an interrupt occurs.
    //
	// Note, as with other timers, if we set the count to, n, then we will
	// interrupt after n+1 clock cycles
	//
	// For a 10 bit write we must write the high byte before the low byte;
	// in this case there is a single shared register for the high bits, TC4H
	// and this is used for writing to ANY 10 bit register, so we need to be
	// aware of what is in there when we write what we might think is an
	// 8 bit value.
	//
	// At 8MHz with a /128 prescaler, one count is equal to 16 us
	// At 194 counts, we will interrupt every 195 periods:
	// at 3.12ms intervals, or 320.51Hz  
	//
	
	cli();		// Disable interrupts

	TIMSK4  = 0x00;		// Disable interrupts for all timer 4 comparisons, plus overflow

	TC4H    = 0x00;		// Reset the timer to zero, high byte first
	TCNT4   = 0x00;		// Reset the timer to zero
						// This must be done first because the TC4H is used to provide
						// high bits for the 10 bit registers accessed below

	TCCR4A  = 0x00;		// Turn off OC4Ax/OC4Bx connections, FfOC and PWM
	TCCR4B  = 0x08;		// No PWM inversion, don't reset the prescaler, don't set dead time, clock/128
    TCCR4C  = 0x00;		// Comparator B and D to normal mode
    TCCR4D  = 0x00;		// Fault protection off
    TCCR4E  = 0x00;		// Don't lock and switch off all output compare pins
	OCR4A   = 0x4F;  	// Set the counter to 194 (0xC2), giving ~320Hz

	TIMSK4 |= 0x40;     // And enable interrupts for a compare A match only

	// Finally, enable the output from the LED drivers
	digitalWrite(LED_R_OE, LOW);
	digitalWrite(LED_G_OE, LOW);
	digitalWrite(LED_B_OE, LOW);
	
	sei();  	// Enable interrupts	
}


/*---------------------------------------------------------------------------*/
/**
* \brief end function - switch off the LEDs
*
* We drive the output enables for all three drivers high. This disconnects
* the driver from the LEDs and so all subsequent changes will have no effect.
* We also stop the timer interrupt.
* 
*/
void EngduinoLEDsClass::end() 
{
	// Disable output from LED drivers
	digitalWrite(LED_R_OE, HIGH);
	digitalWrite(LED_G_OE, HIGH);
	digitalWrite(LED_B_OE, HIGH);

	TIMSK4  = 0x00;		// Disable interrupts for all timer 4 comparisons, plus overflow
}


/*---------------------------------------------------------------------------*/
/**
* \brief Internal function to set an LED to a given colour/brightness.
* \param LEDidx     LED index, ranging from 0 (for LED0) to 15 (LED15)
* \param c          Colour, as chosen from the colour enum
* \param brightness A 0-MAX_BRIGHTNESS(=15) value
*
* Internal function to set the colour/brightness of an LED. The colour value
* here is chosen from an enum and corresponds to the primary and secondary
* colours of light, plus white and off. Brightness ranges from 0 to
* MAX_BRIGHTNESS (currently 15), and is shifted here to an internal 8 bit
* representation (0-255) to make some of the maths slightly quicker later.
* Choosing a brightness of zero will turn an LED off. 
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white is requested, simply because this causes too big a
* drain on current whilst the device is attached to the USB. This
* causes a system reset, which is both irritating and makes reprogramming
* rather tedious.
*
*/
void EngduinoLEDsClass::_setLED(uint8_t LEDidx, colour c, uint8_t brightness)
{	
	LEDidx &= 0x0F;					// We only have 16 LEDS
	brightness = brightness << 4;	// Translate brightness to a 0-255 scale
	
	switch (c) {
		case RED:
			RSet[LEDidx]=brightness; GSet[LEDidx]=0x00;       BSet[LEDidx]=0x00;
			break;
		case GREEN:
			RSet[LEDidx]=0x00;       GSet[LEDidx]=brightness; BSet[LEDidx]=0x00;
			break;
		case BLUE:
			RSet[LEDidx]=0x00;       GSet[LEDidx]=0x00;       BSet[LEDidx]=brightness;
			break;
		case YELLOW:
			RSet[LEDidx]=brightness; GSet[LEDidx]=brightness; BSet[LEDidx]=0x00;
			break;
		case MAGENTA:
			RSet[LEDidx]=brightness; GSet[LEDidx]=0x00;       BSet[LEDidx]=brightness;
			break;
		case CYAN:
			RSet[LEDidx]=0x00;       GSet[LEDidx]=brightness; BSet[LEDidx]=brightness;
			break;
		case WHITE:
			brightness = (brightness > 0xB0) ? 0xB0 : brightness;	// Can't use full scale for white
			RSet[LEDidx]=brightness; GSet[LEDidx]=brightness; BSet[LEDidx]=brightness;		
			break;
		case OFF:
			RSet[LEDidx]=0x00;       GSet[LEDidx]=0x00;       BSet[LEDidx]=0x00;
			break;
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function to set an LED to a given point in the rgb space.
* \param LEDidx     LED index, ranging from 0 (for LED0) to 15 (LED15)
* \param r          Brightness of the red channel from 0-MAX_BRIGHTNESS (15)
* \param g          Brightness of the green channel from 0-MAX_BRIGHTNESS (15)
* \param b          Brightness of the blue channel from 0-MAX_BRIGHTNESS (15)
*
* Internal function to set the colour/brightness of an LED to a point in rgb
* space. The brightness in each channel may range from 0 to MAX_BRIGHTNESS
* (currently 15), and is shifted here to an internal 8 bit representation
* (0-255) to make some of the maths slightly quicker later.
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white, or colours close to white are requested, simply
* because this causes too big a drain on current whilst the device is attached
* to the USB. This causes a system reset, which is both irritating and makes
* reprogramming rather tedious. So we limit the sum of the rgb values to
* a number we chose empirically, and we reduce the chosen rgb values until they
* sum to less than that chosen.
* 
*/
void EngduinoLEDsClass::_setLED(uint8_t LEDidx, uint8_t r, uint8_t g, uint8_t b)
{	

	LEDidx &= 0x0F;		// We only have 16 LEDS
	r = r << 4;			// We only use 16 brightness levels to avoid flickering
	g = g << 4;			// Turn the 0-15 scale into a 0-255 scale; makes maths easier
	b = b << 4;

	#if defined(__BOARD_ENGDUINOV1) || defined(__BOARD_ENGDUINOV2)// Only for version 1 and 2
		// The brightest colours cause a reset on the engduino v1.0
		// when running on USB. So we limit what we allow someone to choose.
		// This subtraction avoids division, which is slow.
		//
		uint16_t sum;
		sum = r + g + b;
		while (sum > 0x240) {	// Empirically determined: (0x24 << 4)
			r -= 1;
			g -= 1;
			b -= 1;
			sum = r + g + b;
		}
	#endif
	
	RSet[LEDidx] = r;
	GSet[LEDidx] = g;
	BSet[LEDidx] = b;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set the colour of a single LED at maximum brightness
* \param LEDNumber  LED number, ranging from 0 (for LED0) to 15 (LED15)
* \param c          Colour, as chosen from the colour enum
*
* Set the colour of an LED. The colour value here is chosen from an enum and
* corresponds to the primary and secondary colours of light, plus white and
* off.
* 
*/
void EngduinoLEDsClass::setLED(uint8_t LEDNumber, colour c)
{	
#ifdef __BOARD_ENGDUINOV1
	LEDNumber--;
#endif
	_setLED(LEDNumber, c, MAX_BRIGHTNESS);

}


/*---------------------------------------------------------------------------*/
/**
* \brief Set an LED to a given colour/brightness.
* \param LEDNumber  LED number, ranging from 0 (for LED0) to 15 (LED15)
* \param c          Colour, as chosen from the colour enum
* \param brightness A 0-MAX_BRIGHTNESS(=15) value
*
* Set the colour of an LED. The colour value here is chosen from an enum and
* corresponds to the primary and secondary colours of light, plus white and
* off. Brightness ranges from 0 to MAX_BRIGHTNESS (currently 15). Choosing a
* brightness of zero will turn an LED off. 
* 
*/
void EngduinoLEDsClass::setLED(uint8_t LEDNumber, colour c, uint8_t brightness)
{
#ifdef __BOARD_ENGDUINOV1
	LEDNumber--;
#endif
	_setLED(LEDNumber, c, brightness);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set an LED to a given point in the rgb space.
* \param LEDNumber  LED number, ranging from 0 (for LED0) to 15 (LED15)
* \param r          Brightness of the red channel from 0-MAX_BRIGHTNESS (15)
* \param g          Brightness of the green channel from 0-MAX_BRIGHTNESS (15)
* \param b          Brightness of the blue channel from 0-MAX_BRIGHTNESS (15)
*
* Set the colour/brightness of an LED to a point in rgb space. The brightness
* in each channel may range from 0 to MAX_BRIGHTNESS (currently 15).
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white, or colours close to white are requested, simply
* because this causes too big a drain on current whilst the device is attached
* to the USB. This causes a system reset, which is both irritating and makes
* reprogramming rather tedious. So we limit the sum of the rgb values to
* a number we chose empirically, and we reduce the chosen rgb values until they
* sum to less than that chosen.
*
*/
void EngduinoLEDsClass::setLED(uint8_t LEDNumber, uint8_t r, uint8_t g, uint8_t b)
{	
#ifdef __BOARD_ENGDUINOV1
	LEDNumber--;
#endif
	_setLED(LEDNumber, r, g, b);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set the colour of all LEDs at maximum brightness
* \param c          Colour, as chosen from the colour enum
*
* Set the colour of all LEDs. The colour value here is chosen from an enum and
* corresponds to the primary and secondary colours of light, plus white and
* off.
*
*/
void EngduinoLEDsClass::setAll(colour c)
{	
	for (int i = 0; i < 16; i++)
		_setLED(i, c, MAX_BRIGHTNESS);
}


/*---------------------------------------------------------------------------*/
/**
* \brief Set all LEDs to a given colour/brightness.
* \param c          Colour, as chosen from the colour enum
* \param brightness A 0-MAX_BRIGHTNESS(=15) value
*
* Set the colour of all LEDs. The colour value here is chosen from an enum and
* corresponds to the primary and secondary colours of light, plus white and
* off. Brightness ranges from 0 to MAX_BRIGHTNESS (currently 15). Choosing a
* brightness of zero will turn all LEDs off. 
*
*/
void EngduinoLEDsClass::setAll(colour c, uint8_t brightness)
{	
	for (int i = 0; i < 16; i++) {
		_setLED(i, c, brightness);
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set all LEDs to a given point in the rgb space.
* \param r          Brightness of the red channel from 0-MAX_BRIGHTNESS (15)
* \param g          Brightness of the green channel from 0-MAX_BRIGHTNESS (15)
* \param b          Brightness of the blue channel from 0-MAX_BRIGHTNESS (15)
*
* Set the colour/brightness of all LEDs to a point in rgb space. The brightness
* in each channel may range from 0 to MAX_BRIGHTNESS (currently 15).
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white, or colours close to white are requested, simply
* because this causes too big a drain on current whilst the device is attached
* to the USB. This causes a system reset, which is both irritating and makes
* reprogramming rather tedious. So we limit the sum of the rgb values to
* a number we chose empirically, and we reduce the chosen rgb values until they
* sum to less than that chosen.
*
*/
void EngduinoLEDsClass::setAll(uint8_t r, uint8_t g, uint8_t b)
{	
	for (int i = 0; i < 16; i++)
		_setLED(i, r, g, b);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set the colour of all LEDs at maximum brightness from an array of
*        individual values
* \param c          Array of colour values, as chosen from the colour enum
*
* Set the colour of all LEDs from an array of colour values - i.e. each LED
* can be set to a different colour, though all are at maximum brightness. The
* colour value here is chosen from an enum and corresponds to the primary and
* secondary colours of light, plus white and off.
*
*/
void EngduinoLEDsClass::setLEDs(colour c[16])
{	
	for (int i = 0; i < 16; i++)
		_setLED(i, c[i], MAX_BRIGHTNESS);
}


/*---------------------------------------------------------------------------*/
/**
* \brief Set all LEDs to a given colour/brightness from arrays of
*        individual values
* \param c          Array of colour values, as chosen from the colour enum
* \param brightness Array of brightness values, from 0-MAX_BRIGHTNESS(=15)
*
* Set the colour/brightness of all LEDs from two arrays of values - i.e.
* each LED can be set to a different colour/brightness value. The colour
* value here is chosen from an enum and corresponds to the primary and
* secondary colours of light, plus white and off. Brightness ranges from 0
* to MAX_BRIGHTNESS (currently 15). Choosing a brightness of zero will turn
* an LED off. 
*
*/
void EngduinoLEDsClass::setLEDs(colour c[16], uint8_t brightness[16])
{	
	for (int i = 0; i < 16; i++) {
		_setLED(i, c[i], brightness[i]);
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set all LEDs to a given point in the rgb space from arrays of
*        individual values.
* \param r          Array of red channel brightness, from 0-MAX_BRIGHTNESS (15)
* \param g          Array of green channel brightness, from 0-MAX_BRIGHTNESS (15)
* \param b          Array of blue channel brightness, from 0-MAX_BRIGHTNESS (15)
*
* Set the colour/brightness of all LEDs to a point in rgb space from three
* arrays of values - i.e. each LED can be set to a different rgb point. The
* brightness in each channel may range from 0 to MAX_BRIGHTNESS (currently 15).
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white, or colours close to white are requested, simply
* because this causes too big a drain on current whilst the device is attached
* to the USB. This causes a system reset, which is both irritating and makes
* reprogramming rather tedious. So we limit the sum of the rgb values to
* a number we chose empirically, and we reduce the chosen rgb values until they
* sum to less than that chosen.
*
*/
void EngduinoLEDsClass::setLEDs(uint8_t r[16], uint8_t g[16], uint8_t b[16])
{	
	for (int i = 0; i < 16; i++)
		_setLED(i, r[i], g[i], b[i]);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set all LEDs to a given point in the rgb space a 2D array of
*        individual values. The array is of form rgb[3][16]
* \param rgb        2D array of rgb brightnesses, from 0-MAX_BRIGHTNESS (15)
*
* Set the colour/brightness of all LEDs to a point in rgb space from a 2D
* array values - i.e. each LED can be set to a different rgb point. The array
* is of form rgb[3][16] - i.e. the first index is the red/green/blue channel
* and the second is the LED (ranging from 0-15). The brightness in each
* channel may range from 0 to MAX_BRIGHTNESS (currently 15).
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white, or colours close to white are requested, simply
* because this causes too big a drain on current whilst the device is attached
* to the USB. This causes a system reset, which is both irritating and makes
* reprogramming rather tedious. So we limit the sum of the rgb values to
* a number we chose empirically, and we reduce the chosen rgb values until they
* sum to less than that chosen.
*
*/
void EngduinoLEDsClass::setLEDs(uint8_t rgb[3][16])
{	
	for (int i = 0; i < 16; i++)
		_setLED(i, rgb[0][i], rgb[1][i], rgb[2][i]);
}


/*---------------------------------------------------------------------------*/
/**
* \brief Set all LEDs to a given point in the rgb space a 2D array of
*        individual values. The array is of form rgb[16][3]
* \param rgb        2D array of rgb brightnesses, from 0-MAX_BRIGHTNESS (15)
*
* Set the colour/brightness of all LEDs to a point in rgb space from a 2D
* array values - i.e. each LED can be set to a different rgb point. The array
* is of form rgb[16][3] - i.e. the first index is the LED (ranging from 0-15).
* and the second is the red/green/blue channel. The brightness in each
* channel may range from 0 to MAX_BRIGHTNESS (currently 15).
* 
* Note: in the Engduino 1.0, we cannot use the full brightness for the LEDs
* when the colour white, or colours close to white are requested, simply
* because this causes too big a drain on current whilst the device is attached
* to the USB. This causes a system reset, which is both irritating and makes
* reprogramming rather tedious. So we limit the sum of the rgb values to
* a number we chose empirically, and we reduce the chosen rgb values until they
* sum to less than that chosen.
*
*/
void EngduinoLEDsClass::setLEDs(uint8_t rgb[16][3])
{	
	for (int i = 0; i < 16; i++)
		_setLED(i, rgb[i][0], rgb[i][1], rgb[i][2]);
}

#if defined(__BOARD_ENGDUINOV1)||defined(__BOARD_ENGDUINOV2)
/*---------------------------------------------------------------------------*/
/**
* \brief This is an internal routine to set the LED latches appropriately
* \param value      The on/off bits for each LED on a given channel
*
* This function is written using low level C programming primitives for
* changing the voltages on ATMega32U4 pins rather than the Arduino
* digitalWrite equivalents. This has to be done in this way for speed - it is
* around 18 times faster than the alternative and, because we update the
* values rather frequently to avoid flickering, speed is important. This code
* manually emulates SPI functionality (mode 0), both for simplicity and speed
* 
* Notes:
* PB1 is pin 9  - SPI SCLK
* PB2 is pin 10 - SPI MOSI
* PB3 is pin 11 - SPI MISO
*
*/
inline void writeLEDs(volatile uint8_t *value)
{
	// This is code to emulate SPI mode 0 = we set the
	// data value first and then tick the clock with a rising
	// edge to latch the data into the slave.
	//
	for (int i = 15; i >= 0; i--) {
		if (value[i] != 0)			// Set the data line...
			PORTB |= _BV(PORTB2); 	// Drive MOSI high
		else
			PORTB &= ~_BV(PORTB2); 	// Drive MOSI low
		PORTB |= _BV(PORTB1); 		// ...and tick the SPI clock
		PORTB &= ~_BV(PORTB1);
	}
}

#elif defined (__BOARD_ENGDUINOV3)
/*---------------------------------------------------------------------------*/
/**
* \brief This is an internal routine to set the LED latches appropriately
* \param value      The on/off bits for each LED on a given channel
*
* This function sends two SPI 8bit packages to simulate 16bit SPI package.
* 
* Notes:
* PB1 is pin 9  - SPI SCLK
* PB2 is pin 10 - SPI MOSI
* PB3 is pin 11 - SPI MISO
*
*/
inline void writeLEDs(volatile uint8_t *value)
{
	uint8_t temp1 = 0;
	uint8_t temp2 = 0;

	for (int i = 15; i >= 8; i--) 
	{
		if (value[i] != 0)	// Check if bit is one
		{
			temp1 = temp1 + (1 << (i-8));
		}
	}


	for (int i = 7; i >= 0; i--) 
	{
		if (value[i] != 0)	// Check if bit is one
		{
			temp2 = temp2 + (1 << i);
		}
	}

	SPI.transfer(temp1);
	SPI.transfer(temp2);
}
#endif

/*---------------------------------------------------------------------------*/
/**
* \brief ISR routine for comparator A of TIMER 4
*
* TIMER4 interrupt code  ticks at ~320Hz, which means we can do 16 brightness
* levels at 20Hz, so flickering is invisible to the naked eye. This ISR
* implements the software PWM and calls the LED display function. 
* 
* Because we must update the LEDs very frequently, and because it still takes
* a significant period to write all 3*16 values to the LED drivers, we need
* this code to run as fast as possible. We also need to ensure that we do not
* prevent other interrupts from occurring whilst we process this. To that end
* we stop interrupts for this ISR to prevent any nesting, reset the counter
* and re-enable interrupts generally before calling setting the LED values.
*
* The PWM implementation relies on counting in units of the brightness on a
* channel and setting the apropriate colour channel on for an LED when the
* accumulated value overflows. This is slightly faster if the rgb values are
* in the range 0-255 than 0-15, because we can do it with a simple comparison.
* 
* This function then calls writeLEDs for each of the RGB values, in reverse
* order. The drivers are daisy chained and we need to shift the blue values
* to the far end of that chain before pulsing the latch. This written using
* low level C programming primitives of changing the voltages on ATMega32U4
* pins rather than the Arduino digitalWrite equivalents. This has to be done
* in this way for speed - it is around 18 times faster than the alternative
* and, because we update the values rather frequently to avoid flickering,
* speed is important.
* 
* Notes:
* The PWM implementation will occasionally put one more dark period
*   in than it should, but we can live with this for simplicity.
* PB7 is pin 12 on the ATMega32U4 and is attached to the LED LATCH
*
*/
ISR(TIMER4_COMPA_vect)
{  
	// Temporarily disable interrupts on this ISR to avoid nesting.
	//
	TIMSK4  = 0x00;

	// Because there is no CTC mode, we need to reset the timer
	// count manually.
	//
	TC4H    = 0x00;		// Reset the timer to zero, high byte first
	TCNT4   = 0x00;		// Reset the timer to zero

	// And re-enable interrupts for all other ISRs whilst we set the LEDs 
	sei();
	

	// Accumulate RGB values for each LED into their respective arrays
	// and only set the LED on when we overflow the count; it is otherwise
	// set off.
	//
	for (int i = 0; i < 16; i++) 
	{
		uint8_t ra = EngduinoLEDs.RAccum[i];
		uint8_t ga = EngduinoLEDs.GAccum[i];
		uint8_t ba = EngduinoLEDs.BAccum[i];
		
		EngduinoLEDs.RAccum[i] += EngduinoLEDs.RSet[i];
		EngduinoLEDs.GAccum[i] += EngduinoLEDs.GSet[i];
		EngduinoLEDs.BAccum[i] += EngduinoLEDs.BSet[i];
		
		EngduinoLEDs.RDisp[i]   = (EngduinoLEDs.RAccum[i] < ra);
		EngduinoLEDs.GDisp[i]   = (EngduinoLEDs.GAccum[i] < ga);
		EngduinoLEDs.BDisp[i]   = (EngduinoLEDs.BAccum[i] < ba);
	}
	
	// Now display the values we calculated.
#ifdef __BOARD_ENDDUINOV1
	PORTB &= ~_BV(PORTB7);			// Drive LATCH low - disable 
#elif defined (__BOARD_ENGDUINOV2)
	PORTB &= ~_BV(PORTB4);	
#elif defined (__BOARD_ENGDUINOV3)
	LED_LATCH_PORT &= ~_BV(LED_LATCH_BIT);			// Drive LATCH low - disable 
#endif
	
	writeLEDs(EngduinoLEDs.BDisp);	// Write the RGB values
	writeLEDs(EngduinoLEDs.GDisp);
	writeLEDs(EngduinoLEDs.RDisp);
#ifdef __BOARD_ENGDUINOV1	
	PORTB |= _BV(PORTB7); 			// Pulse LATCH to switch
	PORTB &= ~_BV(PORTB7);
#elif defined (__BOARD_ENGDUINOV2)
	PORTB |= _BV(PORTB4); 			// Pulse LATCH to switch
	PORTB &= ~_BV(PORTB4);
#elif defined(__BOARD_ENGDUINOV3)	
	LED_LATCH_PORT |= _BV(LED_LATCH_BIT); 			// Pulse LATCH to switch
	LED_LATCH_PORT &= ~_BV(LED_LATCH_BIT);
#endif
	// And we're done with this call of the ISR, so re-enable interrupts
	//
	TIMSK4 |= 0x40;     // And enable interrupts for a compare A match only
}


/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */ 
EngduinoLEDsClass EngduinoLEDs = EngduinoLEDsClass();

/** @} */
