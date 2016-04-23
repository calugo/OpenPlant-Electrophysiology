/**
* \addtogroup EngduinoThermistor
*
* This is the driver code for reading temperature. This can
* equally well be done in a sketch file, but it is also here
* for completeness
* 
* The Engduino has a Murata NCP18WF104J03RB
* thermistor attached to analogue input A5.
* This thermistor has a resistance of 100K at 25C.
*
* @{
*/

/**
* \file 
* 		Engduino Thermistor driver
* \author
* 		Engduino team: support@engduino.org
*/

#include "pins_arduino.h"
#include "EngduinoThermistor.h"

/*
 *  Initialise Class Variables
 */

/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
* 
* C++ constructor for this class. Empty.
*/
EngduinoThermistorClass::EngduinoThermistorClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
*
* Nothing to be done.
*/
void EngduinoThermistorClass::begin() 
{
}


/*---------------------------------------------------------------------------*/
/**
* \brief end function - switch off the LEDs
*
* Nothing to be done.
*/
void EngduinoThermistorClass::end() 
{
}


/*---------------------------------------------------------------------------*/
/**
* \brief Get the temperature.
* \param units A choice between CELSIUS (default), KELVIN, or FAHRENHEIT
* \return The temperature on the given scale.
*
* This function makes a call to the function that actually works out what
* a particular resistance means.
*
* For the Murata NCP18WF104J03RB, the thermistor has a resistance of 100K at
* 25C and a B value of 4250 and a 100K balance resistor.
*/
float EngduinoThermistorClass::temperature(temperatureUnits units)
{
	// Parameters for the Murata NCP18WF104J03RB thermistor
	//
	return (temperature(units, 4250.0, 298.15, 100000.0, 100000.0));
}

/*---------------------------------------------------------------------------*/
/**
* \brief Get the temperature.
* \param units     A choice between CELSIUS (default), KELVIN, or FAHRENHEIT
* \param B				 Parameter in the B equation - see data sheet
* \param T0				 Temperature in Kelvin at which resistance is R0 - see data sheet
* \param R0				 Resistance at temperature T0 - see data sheet
* \param R_Balance Value of the balance resistor in the potential divider (100K on Engduino v1.0)
* \return The temperature on the given scale.
*
* This function measures the voltage and, as a consequence, calculates the
* resistance of the thermistor. From this, we can figure out the actual
* temperature 
* 
* NTC thermistors can be characterised with the B (or beta) parameter equation,
* 
* 1/T = 1/T0 + 1/B*ln(R/R0)
* 
* Where the temperatures are in Kelvin and R0 is the resistance at
* temperature T0 (25 °C = 298.15 K).
* 
* So far as the resistance is concerned, we have a potential divider with a balance
* resistor of value R_Balance. Assuming that the resistance of the NTC is R and the
* voltage across it is V, then Ohms law gives:
* 	V/Vcc = R/(R + R_Balance)
* 	
* Given that V/Vcc is just (analog in)/1024 for a 10 bit ADC, we can work out that:
* 	R = (R_Balance * analogIn) / (1024 - analogIn)
* 
*/
float EngduinoThermistorClass::temperature(temperatureUnits units, float B, float T0, float R0, float R_Balance)
{
	float R, T;
	float analogIn = analogRead(A5);					// Measure the voltage across the NTC. 

	R = (R_Balance * analogIn) / (1024.0 - analogIn);	// Resistance calculated from potential divider
	T = 1.0 / (1.0/T0 + (1.0/B)*log(R/R0));				// Temperature in Kelvin given by B equation
	

	switch (units) {
		case CELSIUS:
			T -= 273.15;
			break;
		case FAHRENHEIT :
			T = (T-273.15)*1.8 + 32.0;
			break;
		default:
			break;
	};
	
	return T;
}


/**
* \brief Get the raw potential divider reading.
* \return Raw potential divider reading.
*
* Raw voltage reading from NTC potential divider circuit
*
*/
uint16_t EngduinoThermistorClass::temperatureRaw()
{
	return(analogRead(NTC));
}


/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */ 
EngduinoThermistorClass EngduinoThermistor = EngduinoThermistorClass();

/** @} */
