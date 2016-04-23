/**
* \defgroup EngduinoLight Driver for Engduino Light Sensor
* 
* @{
*/

/**
* \file 
* 		Engduino Light sensor driver
* \author
* 		Engduino team: support@engduino.org
*/

#ifndef __ENGDUINOLIGHT_H__
#define __ENGDUINOLIGHT_H__

#include <stdio.h>
#include <Arduino.h>
#include <Engduino.h>

#if defined(__BOARD_ENGDUINOV2) || defined(__BOARD_ENGDUINOV3)



/***********************************************************************************************\
* Public prototypes
\***********************************************************************************************/

class EngduinoLightClass 
{
	private:
	
	public:
		EngduinoLightClass();
		void    begin();
		void    end();
		uint16_t lightLevel();

};

extern EngduinoLightClass EngduinoLight;

#endif
#endif
/** @} */
