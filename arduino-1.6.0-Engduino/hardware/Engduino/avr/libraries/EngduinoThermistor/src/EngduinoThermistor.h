/**
* \defgroup EngduinoThermistor Driver for Engduino Thermistor 
* 
* @{
*/

/**
* \file 
* 		Engduino Thermistor driver
* \author
* 		Engduino team: support@engduino.org
*/

#ifndef __ENGDUINOTHERMISTOR_H__
#define __ENGDUINOTHERMISTOR_H__

#include <Arduino.h>
#include <Engduino.h>

enum temperatureUnits {
  CELSIUS,
  KELVIN,
  FAHRENHEIT
};

class EngduinoThermistorClass 
{
	private:

	public:
		EngduinoThermistorClass();
		void     begin();
		void     end();
		float    temperature(temperatureUnits units=CELSIUS);
		float    temperature(temperatureUnits units, float B, float T0, float R0, float R_Balance);
		uint16_t temperatureRaw();
};

extern EngduinoThermistorClass EngduinoThermistor;

#endif

/** @} */
