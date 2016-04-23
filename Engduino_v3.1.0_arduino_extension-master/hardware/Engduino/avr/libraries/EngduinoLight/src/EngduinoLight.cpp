/**
* \addtogroup EngduinoLight
*
* This is the driver code for Light on the Engduino
* In v3.0 this light sensor is a Avago ADPS-9008.
*
* @{
*/

/**
* \file 
* 		Engduino Light Sensor driver
* \author
* 		Engduino team: support@engduino.org
*/


#include "pins_arduino.h"
#include "EngduinoLight.h"

#if defined(__BOARD_ENGDUINOV2) || defined(__BOARD_ENGDUINOV3)

/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
* 
* C++ constructor for this class. Empty.
*/
EngduinoLightClass::EngduinoLightClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
*
* Does nothing in this case
*/
void EngduinoLightClass::begin() 
{	
}

/*---------------------------------------------------------------------------*/
/**
* \brief end function - switch off the light sensor
*
* Does nothing in this case
*/
void EngduinoLightClass::end() 
{	
}

/*---------------------------------------------------------------------------*/
/**
* \brief lightLevel function - returns light level (10 bit value)
*
* This function returns the light level. Since the input is an analogue one
* and the ADC gives a 10 bit value, it returns a 16 bit integer.
*  
*/
uint16_t EngduinoLightClass::lightLevel() 
{	
	return analogRead(LIGHTSENSOR);
}

/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */ 
EngduinoLightClass EngduinoLight = EngduinoLightClass();

/** @} */
#endif