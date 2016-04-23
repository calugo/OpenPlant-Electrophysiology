/**
* \addtogroup EngduinoSD
*
* This library aims to expose a subset of SD card functionality
* in the form of a higher level "wrapper" object.
*
* The Engduino has a micro SD card slot. Maximum size of attached SD card is 4GB.
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
#include "EngduinoSD.h"

#define DEBUGG 0
#if DEBUGG
	#define PRINTLN(...) Serial.println(__VA_ARGS__)
#else
	#define PRINTLN(...)
#endif

#ifdef __BOARD_ENGDUINOV3
/*
 *  Initialise Class Variables
 */

/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
*
* C++ constructor for this class. Empty.
*/
EngduinoSDClass::EngduinoSDClass()
{
	initialized = false;
	opened = false;
	autoOpenClose = false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
* \return True if initialization succeeds, false otherwise..
*
* This function performs the initialisation required by the sdfatlib library.
*/
boolean EngduinoSDClass::begin()
{
	PRINTLN("begin() init");
	if(initialized) return true; // If we are already initialized, return true.
	if(!isAttached()) return false;
	// make sure that the default chip select pin is set to
  	// output, even if you don't use it:
  	pinMode(SPI_SS_PIN, OUTPUT);
	regTmp = TIMSK4;
	TIMSK4  = 0x00;	// Disable TMR4 interrupts (Used by LEDs).
	PRINTLN("SD.begin...");
	boolean b = SD.begin(SDCARD_CS);
	TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
	if(b) {initialized = true; PRINTLN("SD.begin...success"); return true;}
    return false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
* \return True if initialization succeeds, false otherwise..
*
* This function performs the initialisation required by the sdfatlib library.
*/
boolean EngduinoSDClass::begin(const char *filepath, uint8_t mode)
{
	PRINTLN("begin(..., ...)");
	if(initialized) return true; // If we are already initialized, return true.
	if(!isAttached()) return false;
  	// make sure that the default chip select pin is set to
  	// output, even if you don't use it:
  	pinMode(SPI_SS_PIN, OUTPUT);
	regTmp = TIMSK4;
	TIMSK4  = 0x00;	// Disable TMR4 interrupts (Used by LEDs).
	PRINTLN("SD.begin");
	if(SD.begin(SDCARD_CS))
	{
		PRINTLN("SD.open");
		file = SD.open(filepath, mode);
		if(file) // if the file is available return true
		{
			PRINTLN("File is available.");
			strncpy(filePath, filepath, 12); // Maximum length of file name is 8.
			filePath[12] = 0;
			fileMode = mode;
			file.close();
			TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
			initialized = true;
			opened = true;
			autoOpenClose = true;
			return true;
		}
		else
		{
			PRINTLN("File is not available!");
		}
	}
	initialized = false;
	opened = false;
	autoOpenClose = false;
	TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
    return false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief open function
* \return True if file open succeeds, false otherwise..
*
* This function is used to open a file on the SD card.
*/
boolean EngduinoSDClass::open(const char *filepath, uint8_t mode)
{
	PRINTLN("open(..., ...)");
	if(!isAttached()) return false;
	regTmp = TIMSK4;
	TIMSK4  = 0x00;	// Disable TMR4 interrupts (Used by LEDs).
	PRINTLN("SD.open");
	file = SD.open(filepath, mode);
	if(file) { // if the file is available return true
		PRINTLN("File is available.");
		strncpy(filePath, filepath, 12); // Maximum length of file name is 8.
		filePath[12] = 0;
		fileMode = mode;
		opened = true;
		autoOpenClose = false;
		TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
		return true;
	}
	else 
	{
		PRINTLN("File is not available!");
	}
	opened = false;
	TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
    return false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief close function
* \return True if file open succeeds, false otherwise..
*
* This function is used to close a file on the SD card.
*/

boolean EngduinoSDClass::close()
{
	PRINTLN("close()");
	if(!isAttached()) return false;
	regTmp = TIMSK4;
	TIMSK4  = 0x00;	// Disable TMR4 interrupts (Used by LEDs).
	if(file) // if the file is available return true
	{
		file.close();
		TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
		opened = false;
		return true;
	}
	TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
    return false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief write function
*
* This function is used to write string to the file.
*/

boolean EngduinoSDClass::write(const String &str)
{
	return _write(str, false);
}

/*---------------------------------------------------------------------------*/
/**
* \brief writeln function
*
* This function is used to write string to the file with "goto new line" (\n) character.
*/

boolean EngduinoSDClass::writeln(const String &str)
{
	return _write(str, true);
}

/*---------------------------------------------------------------------------*/
/**
* \brief write function
*
* This function is used to write a file on the SD card.
* If the input parameter "ln" is true then add "goto new line" (\n) character.
*/

boolean EngduinoSDClass::_write(const String &str, boolean ln)
{
	PRINTLN("write(...)");
	if(fileMode != FILE_WRITE) return false;
	if(!isAttached()) return false;
	if(!initialized) return false;
	if(!opened) return false;
	regTmp = TIMSK4;
	TIMSK4  = 0x00;	// Disable TMR4 interrupts (Used by LEDs).
	if(autoOpenClose)
	{
		file = SD.open(filePath, fileMode);
		if(file)
		{
			(ln) ? file.println(str) : file.print(str);
			file.close();
			TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
			return true;
		}
	}
	else
	{
		if(file)
		{
			(ln) ? file.println(str) : file.print(str);
			TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
			return true;
		}
	}
	TIMSK4 = regTmp;  // Enable TMR4 interrupts (Used by LEDs).
	return false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief available function
*
* This function is used to check if there is still data to be read from file.
*/

int EngduinoSDClass::available()
{
	if(!file) {
		PRINTLN("available(...) file error!");
		return 0; 
	}
	return file.available();
}

/*---------------------------------------------------------------------------*/
/**
* \brief read function
*
* This function is used to read one character from opened file.
*/

uint8_t EngduinoSDClass::read()
{
	if(fileMode != FILE_READ)  {
		PRINTLN("read(...) fileMode != FILE_READ");
		return 0; 
	}
	if(!file) {
		PRINTLN("read(...) file error!");
		return 0; 
	}
	return file.read();
}

/*---------------------------------------------------------------------------*/
/**
* \brief isAttached function
*
* This function is used to check if the SD card is plugged in.
*/

boolean EngduinoSDClass::isAttached()
{
	PRINTLN("isAttached(...)");
	// Read the input on analog pin SDCARD_ATTACHED:
	int analogIn = analogRead(SDCARD_ATTACHED); // Goes from 0 - 1023
	if(analogIn < 300) {PRINTLN("Attached"); return true;} // less than ~1V
	PRINTLN("NOT Attached");
	return false;
}

/*---------------------------------------------------------------------------*/
/**
* \brief isInitialized function
*
* This function is used to return the initialization status of the SD card.
*/

boolean EngduinoSDClass::isInitialized()
{
	return initialized;
}

/*---------------------------------------------------------------------------*/
/**
* \brief end function
*
* Nothing to be done.
*/
void EngduinoSDClass::end()
{
}


/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */
EngduinoSDClass EngduinoSD = EngduinoSDClass();

/** @} */
#endif
