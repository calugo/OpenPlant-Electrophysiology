/**
* \defgroup EngduinoSD Driver for Engduino SD card 
* 
* @{
*/

/**
* \file 
* 		Engduino SD card driver
* \author
* 		Engduino team: support@engduino.org
*/

#ifndef __ENGDUINOSD_H__
#define __ENGDUINOSD_H__


#include <Arduino.h>
#include <Engduino.h>
#include <SD.h>
#ifdef __BOARD_ENGDUINOV3
class EngduinoSDClass 
{
	protected:
		volatile unsigned char regTmp;

	private:
		File file; // Main file. Only one file can be open at a time.
		char filePath[13];
		uint8_t fileMode;
		boolean initialized;
		boolean opened;
		boolean autoOpenClose;

		boolean _write(const String &, boolean ln);

	public:
		EngduinoSDClass();
		boolean     begin();
		boolean		begin(const char *filepath, uint8_t mode);
		boolean		open(const char *filepath, uint8_t mode); 
		boolean		close();  
		void     	end();
		boolean 	write(const String &);
		boolean 	writeln(const String &);
		boolean		isAttached();
		boolean		isInitialized();
		int			available();
		uint8_t		read();
		//uint8_t*	readln(); TODO: need to implement
};

extern EngduinoSDClass EngduinoSD;

#endif
#endif

/** @} */
