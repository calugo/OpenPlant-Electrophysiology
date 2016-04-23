/**
* \addtogroup EngduinoProtocol
*
* This is the driver code for the Engduino protocol. 

* The Engduino protocol enables communication between Engduino
* board and external devices such as personal computer or 
* smart phone/tablet over the small set of instructions/commands. 
* Commands are represented by easy to understand ASCII 
* strings with predefined definitions of different parameters. 
* Commands can be send over the terminal which supports UART 
* communication e.g. Putty, HyperTerminal, Matlab, etc or
* over the Bluetooth communication. Bluetooth communication 
* can be established between Engduino Bluetooth extension 
* board and a Smart device (mobile phone, tablet, etc) or 
* personal computer with Bluetooth port. In the case of 
* Bluetooth communication Bluetooth extension board for 
* Engduino need to be used. 
* 
* The main sketch implementing this protocol:
* ..\examples\100.Engduino\Protocol\Protocol.ino
*
* Engduino package:
* Start chr, delimiter, end chr
*    {          ;         }
*
* Simplest Engduino package:
* {PACKAGE_TYPE; COMMAND_ID; VAL1, ..., VALn}
*
* Examples:
* {1;10;1} or with defines {PACKAGE_TYPE_1; COM_SET_LEDS; GREEN} 
* -> Turns on all RGB LEDs with green colour. 
*
* {1;100} or with defines {PACKAGE_TYPE_1; COM_GET_VERSION} 
* -> Returns package as {1;100;VERSION} 
*
* {1;111;100} or with defines {PACKAGE_TYPE_1; COM_GET_TEMPERATURE; 100} 
* -> Enables continuous mode on Temperature sensor. Package with
*    measurement of temperature will be sends back on every 100 
*    milliseconds. Sample: {1;111;27546} represents 27.546°C.
*   
* @{
*/

/**
* \file
* 		Engduino Protocol driver
* \author
* 		Engduino team: support@engduino.org
*		Engduino site: http://www.engduino.org/
*/

#include "pins_arduino.h"
#include "EngduinoProtocol.h"

// Enable debug option for the debugging proposes. Note that the
// same communication channel e.g. COM5 is used for protocol 
// commands and for the debug messages!
#define DEBUGG 0
#if DEBUGG
	#define PRINTLN(...) Serial.println(__VA_ARGS__)
	#define PRINT(...) Serial.print(__VA_ARGS__)
#else if
	#define PRINTLN(...)
	#define PRINT(...)
#endif

/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
*
* C++ constructor for this class. Empty.
*/
EngduinoProtocolClass::EngduinoProtocolClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief Destructor
*
* C++ destructor for this class. Empty.
*/
EngduinoProtocolClass::~EngduinoProtocolClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
* \param eledsc		Pointer to the EngduinoLEDsClass object
* \param eaccc      Pointer to the EngduinoAccelerometerClass object
* \param etc        Pointer to the EngduinoThermistorClass object
* \param emc        Pointer to the EngduinoMagnetometerClass object
* \param elc        Pointer to the EngduinoLightClass object
* \param ebc        Pointer to the EngduinoButtonClass object
* \param eirc       Pointer to the EngduinoIRClass object
*
* This function initialize Engduino protocol functionalities. It needs
* to be called with all input parameters representing Engduino objects.
* Objects itself needs to be initialized in user code with required 
* users definitions. 
*/
void EngduinoProtocolClass::begin(EngduinoLEDsClass *eledsc,
								  EngduinoAccelerometerClass *eaccc,
							      EngduinoThermistorClass *etc,
							      EngduinoMagnetometerClass *emc,
							      EngduinoLightClass *elc,
							      EngduinoButtonClass *ebc,
								  EngduinoIRClass *eirc)
{
	PRINTLN("PROTOCOL begin");
	commChannel = -1;

	// Initialize small green LED.
	pinMode(LED1, OUTPUT); 

	// Copy object pointers to local variables.
	engduinoLEDs =			eledsc;
	engduinoAccelerometer = eaccc;
	engduinoThermistor =	etc;
	engduinoMagnetometer =	emc;
	engduinoLight =			elc;
	engduinoButton =		ebc;
	engduinoIR =			eirc;

	// Timer 1 initialization.
	cli();									// Disable interrupts.
    TCCR1B |= 1<<CS10 | 1<<CS11 | 1<<WGM12; // Set up timer with prescaler = 64 and CTC mode.
	TIMSK1 |= (1 << OCIE1A);				// Enable compare interrupt.
    TCNT1 = 0; 								// Initialize counter.
    OCR1A = 1000; 							// Initialize compare value.
    sei();									// Enable interrupts.

	// Load default settings.
	loadDefaults();
}

/*---------------------------------------------------------------------------*/
/**
* \brief end function - turn off the LEDs and stop Protocol OS.
*
* Stop the timer 1 interrupt.
*
*/
void EngduinoProtocolClass::end()
{
	// Disable output from LED drivers.
	digitalWrite(LED_R_OE, HIGH);
	digitalWrite(LED_G_OE, HIGH);
	digitalWrite(LED_B_OE, HIGH);

	// Disable interrupts for all timer 1 comparisons, plus overflow.
	// This will stops Protocol OS.
	TIMSK1  = 0x00;		
}

/*---------------------------------------------------------------------------*/
/**
* \brief Initialize protocol variables and settings
*
* Load default settings.
*
*/
void EngduinoProtocolClass::loadDefaults()
{
	// Number of samples per second.
	overSampling = 0; 

	// Initialize sensors buffers.
	for(byte i = 0; i < NR_SENSORS; i++)
	{
		sensorsSamples[i] = 0;
	}

	for(byte i = 0; i < NR_SENSORS_BUF; i++)
	{
		sensorsSum[i] = 0;
	}

	// Initialize oversampling variables.
	overSamplingCnt = 0;
	overSamplingMatch = 0;
	overSamplingRead = 0;

	for(byte i = 0; i < NR_SENSORS; i++)
	{
		sensorsMatch[i] = -1;
		sensorsCnt[i] = 0;
		sensorsReadOnce[0] = 0;
	}

	// Initialize button state.
	buttonCatchEnable[BUTTON_PRESSED] = false;
	buttonCatchEnable[BUTTON_RELEASED] = false;

	// Initialize IR receive/send buffer.
	for(byte i = 0; i < IRBUFSZ; i++)
	{
		irBuf[i] = 0;
	}

	cntt = 0;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Splits string into array of values.
* \param str		Input string representing Engduino package
* \param outNrVals  Pointer to an output variable which holds the value of
*					splited strings into values type long
* \param outVals	Pointer to an output array with values type long
* \return			Unconditional RES_OK 
*
* This function will split Engduino package into array of values type long.
* String will be splitted by delimiter and then converted into long numbers.
* The number of splitted numbers will be also returned.
*
* Example of Engduino package: {1;10;123} as an input parameter "str".
*
*/
int EngduinoProtocolClass::parsePackage(String str, byte *outNrVals, long *outVals)
{
	byte cnt = 0;
	int indx = 0;
	int oldIndx = 0;
	String sval;

	PRINTLN("Package received: " + str);

	// Add final delimiter.
	str.concat(PACKAGE_DELIMITER_CHR); 
	for(int i = 0; i < PACKAGE_MAXNR_VALUES; i++)
	{
		indx = str.indexOf(PACKAGE_DELIMITER_CHR, oldIndx);
		if(indx == -1) 
		{
			// No more strings. Break and return.
			break; 
		}

		// Get index of the next substring.
		sval = str.substring(oldIndx, indx);
		oldIndx = indx+1;

		// Convert string into a number of type long.
		outVals[i] = sval.toInt();
		cnt++;
	}

	// Copy local values count variable into returning one.
	*outNrVals = cnt;

	PRINTLN(cnt);
	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Parse package header
* \param communicationChannel	Communication channel [PC_TERMINAL or BT_MODULE]
* \param inNrVals				Number of input values of type long
* \param inVals					Array of input values of type long
* \param outPackage				Pointer to an output Engduino package structure
* \param outHeaderSize			Pointer to an output variable holding the length of 
*								header. Header size depends on the package type.
* \return						On success RES_OK, otherwise error code represents
*								particular error.
*
* This function parse input array of values into the Engduino package structure. The
* first value in array always represents the package type. Other values in header 
* depend on the package type. The simplest package type is PACKAGE_TYPE_1, which 
* includes only a command ID. Other package types are more advanced including 
* additional header informations like address, timestamp, acknowledge request, etc. 
*
*/
int EngduinoProtocolClass::parsePackageHeader(byte communicationChannel, byte inNrVals, long *inVals, struct EngduinoPackage *outPackage, byte *outHeaderSize)
{
	// Update communication channel. This channel will be used for the next send operation.
	// Protocol OS can handle both type of communications, but not at the same time. However
	// packages can be received from both channels in sequence and responded accordingly to
	// the latest communication channel. This functionalities enables controlling of Engduino 
	// device from PC and smart device at the same session.
	commChannel = communicationChannel;

	if(inNrVals > 0)
	{
		switch (inVals[PACKAGE_PACKAGE_TYPE])
		{
			// Simplest package version
			case PACKAGE_TYPE_1:
				if(inNrVals < 2) return RES_ERR_PACKAGE_HEADER;
				outPackage->packageType = PACKAGE_TYPE_1;
				outPackage->commandID = (byte)inVals[1];
				*outHeaderSize = 2;
				break;
			
			// Advanced package version
			case PACKAGE_TYPE_2:
				if(inNrVals < 4) return RES_ERR_PACKAGE_HEADER;
				outPackage->packageType = PACKAGE_TYPE_1;
				outPackage->commandID = (byte)inVals[1];
				outPackage->packageID = (long)inVals[2];
				outPackage->ack = 		(byte)inVals[3];
				*outHeaderSize = 4;
				break;
			default:
				return RES_ERR_PACKAGE_HEADER;
		}
	}
	else
	{
		return RES_ERR_PACKAGE_HEADER;
	}

	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Send package over the active communication channel.
* \param inNrVals			Number of input values of type long
* \param inCommandID		Command package ID
* \param inVals				Array of input values of type long
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function converts array of input variables into a string and sends 
* package over the active communication channel (Serial or BT). The simplest
* package version is used.
*
* Example of output format: {a;b;c;...x;y;z}
*
*/
int EngduinoProtocolClass::sendPackage(byte inNrVals, byte inCommandID, long *inVals)
{
	if(commChannel == COMMUNICATION_PC_TERMINAL)
	{
		// Communication over the Serial port.
		PRINTLN("sendPackage: COMMUNICATION_PC_TERMINAL");
		Serial.print(PACKAGE_START_CHR);
		Serial.print(PACKAGE_TYPE_1);
		Serial.print(PACKAGE_DELIMITER_CHR);
		Serial.print(inCommandID);
		for(byte i=0; i<inNrVals; i++)
		{
			Serial.print(PACKAGE_DELIMITER_CHR);
			Serial.print(inVals[i]);
		}
		Serial.print(PACKAGE_STOP_CHR);
	}
	else if(commChannel == COMMUNICATION_BT_MODULE)
	{
		// Communication over the Bluetooth port.
		PRINTLN("sendPackage: COMMUNICATION_BT_MODULE");
		Serial1.print(PACKAGE_START_CHR);
		Serial1.print(PACKAGE_TYPE_1);
		Serial1.print(PACKAGE_DELIMITER_CHR);
		Serial1.print(inCommandID);
		for(byte i=0; i<inNrVals; i++)
		{
			Serial1.print(PACKAGE_DELIMITER_CHR);
			Serial1.print(inVals[i]);
		}
		Serial1.print(PACKAGE_STOP_CHR);
	}
	else
	{
		return RES_ERR;
	}
	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Send package over the active communication channel.
* \param engPackage			Pointer to an Engduino package structure
* \param inNrVals			Number of input values of type long
* \param inCommandID		Command package ID
* \param inVals				Array of input values of type long
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function converts array of input variables into a string and sends 
* package over the active communication channel (Serial or BT). Header fields 
* will be added accordingly to the package type. Package
* Format: {a,b,c,...x,y,z}
*
*/
int EngduinoProtocolClass::sendPackage(struct EngduinoPackage *engPackage, byte inNrVals, byte inCommandID, long *inVals)
{
	if(commChannel == COMMUNICATION_PC_TERMINAL) // COMMUNICATION_PC_TERMINAL
	{
		PRINTLN("sendPackage: COMMUNICATION_PC_TERMINAL");
		Serial.print(PACKAGE_START_CHR);
		switch (engPackage->packageType)
		{
			case PACKAGE_TYPE_1:
				Serial.print(engPackage->packageType);
				Serial.print(PACKAGE_DELIMITER_CHR);
				Serial.print(inCommandID);
				break;
			case PACKAGE_TYPE_2:
				Serial.print(engPackage->packageType);
				Serial.print(PACKAGE_DELIMITER_CHR);
				Serial.print(inCommandID);
				Serial.print(PACKAGE_DELIMITER_CHR);
				Serial.print(engPackage->packageID);
				Serial.print(PACKAGE_DELIMITER_CHR);
				Serial.print(engPackage->ack);
				break;
			default:
				return RES_ERR_PACKAGE_HEADER;
		}

		for(byte i=0; i<inNrVals; i++)
		{
			Serial.print(PACKAGE_DELIMITER_CHR);
			Serial.print(inVals[i]);
		}
		Serial.print(PACKAGE_STOP_CHR);
	}
	else if(commChannel == COMMUNICATION_BT_MODULE) // COMMUNICATION_BT_MODULE
	{
		PRINTLN("sendPackage: COMMUNICATION_BT_MODULE");
		Serial1.print(PACKAGE_START_CHR);
		switch (engPackage->packageType)
		{
			case PACKAGE_TYPE_1:
				Serial1.print(engPackage->packageType);
				Serial1.print(PACKAGE_DELIMITER_CHR);
				Serial1.print(inCommandID);
				break;
			case PACKAGE_TYPE_2:
				Serial1.print(engPackage->packageType);
				Serial1.print(PACKAGE_DELIMITER_CHR);
				Serial1.print(inCommandID);
				Serial1.print(PACKAGE_DELIMITER_CHR);
				Serial1.print(engPackage->packageID);
				Serial1.print(PACKAGE_DELIMITER_CHR);
				Serial1.print(engPackage->ack);
				break;
			default:
				return RES_ERR_PACKAGE_HEADER;
		}

		for(byte i=0; i<inNrVals; i++)
		{
			Serial1.print(PACKAGE_DELIMITER_CHR);
			Serial1.print(inVals[i]);
		}
		Serial1.print(PACKAGE_STOP_CHR);
	}
	else
	{
		return RES_ERR;
	}
	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Turn on or off small green LED
* \param engPackage			Pointer to an Engduino package structure
* \param inNrVals			Number of input values
* \param inVals				Array of input values
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function parses input value and turn on of off small green LED.
*
* Input value examples:
* [0   ] -> Turn off LED
* [>= 1] -> Turn on LED
*
*/
int EngduinoProtocolClass::setLED(struct EngduinoPackage *engPackage, byte inNrVals, long *inVals)
{
	if(inNrVals == 1)
	{
		(inVals[0]) ? digitalWrite(LED1, HIGH) : digitalWrite(LED1, LOW);
		return RES_OK;
	}
	else
	{
		return RES_ERR_PACKAGE_NR_VALS;
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set colours and brightness of RGB LEDs  
* \param engPackage			Pointer to an Engduino package structure
* \param inNrVals			Number of input values
* \param inVals				Array of input values
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function parses input values into the RGB colours and brightness. Function 
* supports many different options how to manipulate with LEDs. From turning on a 
* single LED to a full RGB specter on each LED.
*
* Input value examples:
* NrOfParameters							Value								Action
* 1  - [colour]								[0-7]								Set colour of all LEDs.
* 2  - [ledID, colour]						[0-15, 0-7]							Set colour of one LED.	
* 3  - [cR, cG, cB]							[0-15, 0-15, 0-15]					Set RGB colour of all LEDs.
* 4  - [ledID, cR, cG, cB]					[0-7, 0-15, 0-15, 0-15]				Set RGB colour of one LED.
* 16 - [c0, c1, ... , c15]					[0-15, 0-15, ..., 0-15]				Set colours specific colours of all LEDs.
* 32 - [c0, ..., c15, b0, ..., b15]			[0-7, ..., 0-7, 0-15, ..., 0-15]	Set specific colours and brightness of all LEDs.
* 48 - [r0, g0, b0, ... , r15, g15, b15]	[0-15, ..., 0-15]					Set specific RGB colours of all LEDs.
*
*/
int EngduinoProtocolClass::setLEDs(struct EngduinoPackage *engPackage, byte inNrVals, long *inVals)
{
	if(inNrVals == 1)		
	{
		// Set colour of all LEDs.
		engduinoLEDs->setAll((colour)inVals[0]);
		return RES_OK;
	}
	else if(inNrVals == 2)		
	{
		// Set colour of one LED.
		engduinoLEDs->setLED((uint8_t)inVals[0], (colour)inVals[1]);
		return RES_OK;
	}
	else if(inNrVals == 3)		
	{
		// Set RGB colour of all LEDs.
		engduinoLEDs->setLED((uint8_t)inVals[0], (colour)inVals[1], (uint8_t)inVals[2]);
		return RES_OK;
	}
	else if(inNrVals == 4)		
	{
		// Set RGB colour of one LED.
		engduinoLEDs->setLED((uint8_t)inVals[0], (uint8_t)inVals[1], (uint8_t)inVals[2], (uint8_t)inVals[3]);
		return RES_OK;
	}
	else if(inNrVals == 16)		
	{
		// Set colours specific colours of all LEDs.
		for(int i=0; i<16; i++) ledsColor[i] = (colour)inVals[i];
		engduinoLEDs->setLEDs(ledsColor);
		return RES_OK;
	}
	else if(inNrVals == 32)	
	{
		// Set specific colours and brightness of all LEDs.
		for(int i = 0; i < 16; i++)
		{
			ledsColor[i] = (colour)inVals[i];
			ledsBrightness[i] = (uint8_t)inVals[i+16];
		}
		engduinoLEDs->setLEDs(ledsColor, ledsBrightness);
		return RES_OK;
	}
	else if(inNrVals == 48)
	{
		// Set specific RGB colours of all LEDs.
		for(int i = 0; i < 16; i++)
		{
			ledsRGB[0][i] = (uint8_t)inVals[(i*3) + 0]; // Red
			ledsRGB[1][i] = (uint8_t)inVals[(i*3) + 1]; // Green
			ledsRGB[2][i] = (uint8_t)inVals[(i*3) + 2]; // Blue
		}
		engduinoLEDs->setLEDs(ledsRGB);
		return RES_OK;
	}
	else
	{
		return RES_ERR_PACKAGE_NR_VALS;
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set or Get data from IR communication 
* \param engPackage			Pointer to an Engduino package structure
* \param inNrVals			Number of input values
* \param inVals				Array of input values
* \param setGet				Set or Get flag
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function will sends data over the IR communication or wait on data to 
* be received. Timeout is specified by input parameter. After the timeout 
* expiries received data will be send back over the active communication
* channel.
*
* Input value examples: SET (setGet = 0)
* [69,110,103,100,117,105,110,111] -> "Engduino"
* Array "inVals" represents letters (ASCII values; http://www.asciitable.com/) 
* separated by delimiter.
*
* Input value examples: GET (setGet = 1)
* [1000] -> Timeout is set on 1000us (1ms); Max timeout is 65535us
*
*/
int EngduinoProtocolClass::setGetIR(struct EngduinoPackage *engPackage, byte inNrVals, long *inVals, uint8_t setGet)
{
	int j = 0;;
	if(setGet == 0) 
	{
		// Set
		if(inNrVals < IRBUFSZ)
		{
			for(byte i = 0; i < inNrVals; i++)
			{
				irBuf[j++] = (uint8_t)(inVals[i]);
			}
			irBuf[j++] = 0;
			engduinoIR->send(irBuf, j);
			return RES_OK;
		}
		else
		{
			return RES_ERR_PACKAGE_NR_VALS;
		}

	}
	else	
	{
		// Get
		int len = 0;
		uint8_t  buf[IRBUFSZ];

		if(inNrVals == 1)
		{
			// Wait and read the message, if there are some data. Blocking code!
			len = EngduinoIR.recv(buf, (uint16_t)(inVals[0]));

			if (len < 0) len = 0;
			for(byte i = 0; i < len; i++)
			{
				vals[i] = (long)(buf[i]);
			}
			sendPackage(engPackage, len, COM_GET_IR, vals);
			return RES_OK;
		}
		else
		{
			return RES_ERR_PACKAGE_NR_VALS;
		}
	}
	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Set or Get status of the Engduino board 
* \param engPackage			Pointer to an Engduino package structure
* \param inNrVals			Number of input values
* \param inVals				Array of input values
* \param setGet				Set or Get flag
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function will sets or gets Engduino configuration. Parameters needs
* to be specified in KEY-VALUE pairs. Multiple pairs can be included in the 
* same package.
*
* Input value examples: SET (setGet = 0)
* [0,12, 1,23, 6,0, ...] -> Set status variable 0 to a value 12, Set status 
* variable 1 to a value 23, Set status variable 6 to a value 0.
*
* Input value examples: GET (setGet = 1)
* [0, 1, 6] -> Will return KEY-VALUE pairs for status variables 0, 1 and 6.
* E.g.: [0,12, 1,23, 6,0]
*
*/
int EngduinoProtocolClass::setGetStatus(struct EngduinoPackage *engPackage, byte inNrVals, long *inVals, uint8_t setGet)
{
	int nrOf;
	if(setGet == 0) 
	{
		// Set
		// Number of input parameters must be even.
		if(inNrVals % 2) return RES_ERR_PACKAGE_NR_VALS;
		nrOf = inNrVals/2;
		for(int i = 0; i < nrOf; i++)
		{
			switch (inVals[i*2]) // Key
			{
				case STATUS_OVERSAMPLING: // Number of samples per second
					overSampling = (uint16_t)inVals[(i*2)+1]; // Value
					if ((uint16_t)(overSampling * TIME_STAMP) > 1000) overSampling = (uint16_t)(1000 / TIME_STAMP);
					overSamplingMatch = (uint16_t)(1000 / (overSampling * TIME_STAMP));
					for(byte j = 0; j < NR_SENSORS_BUF; j++) sensorsSum[j] = 0;
					for(byte j = 0; j < NR_SENSORS; j++) sensorsSamples[j] = 0;
					overSamplingCnt = 0;
					overSamplingRead = 0;
					break;
				default:
					return RES_ERR_PACKAGE_UNKNOWN_STATUS_KEY;
			}
		}
	}
	else	
	{
		// Get
		nrOf = inNrVals;
		for(int i = 0; i < nrOf; i++)
		{
			// Key
			switch (inVals[i]) 
			{
				// Number of samples per second.
				case STATUS_OVERSAMPLING: 
					vals[(i*2)+0] = (long)STATUS_OVERSAMPLING;	// Key
					vals[(i*2)+1] = (long)overSampling;			// Value
					break;
				default:
					return RES_ERR_PACKAGE_UNKNOWN_STATUS_KEY;
			}
		}
		sendPackage(engPackage, nrOf*2, COM_GET_STATUS, vals);
	}
	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Get Engduino board version and running sketch on it.
* \param engPackage			Pointer to an Engduino package structure
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function will sends back version of the Engduino board and running 
* sketch on it.
*
*/
int EngduinoProtocolClass::getVersion(struct EngduinoPackage *engPackage)
{
	vals[0] = VERSION;
	sendPackage(engPackage, 1, COM_GET_VERSION, vals);
	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Get Engduino's sensor measurements
* \param engPackage			Pointer to an Engduino package structure
* \param sensorType			Chose from which sensor
* \param inNrVals			Number of input values
* \param inVals				Array of input values
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function will sends back measurements from chosen on-board sensor
* or multiple sensors at once. It also supports continuous sampling with 
* specified sampling interval. Active sensor is defined by "sensorType" 
* variable e.g.: SENSOR_TEMP, SENSOR_ACC, SENSOR_ALL, ...
*
* Input value examples:
* [0 ] -> Read once
* [-1] -> Stop continuous sampling if it is active
* [>0] -> Start continuous sampling with time interval in [ms] 
*
*/
int EngduinoProtocolClass::getSensor(struct EngduinoPackage *engPackage, int sensorType, byte inNrVals, long *inVals)
{
	if(inNrVals > 1) return RES_ERR_PACKAGE_NR_VALS;
	if(sensorType >= NR_SENSORS) return RES_ERR_PACKAGE_UNKNOWN_SENSOR_TYPE;
	if(inNrVals == 1 && inVals[0] == -1) 
	{
		// Stop reading.
		sensorsMatch[sensorType] = -1;
		return RES_OK;
	}
	else if(inNrVals == 1 && inVals[0] > 0) 
	{
		// set continuous reading interval.
		if(inVals[0] < MIN_TIME) inVals[0] = MIN_TIME;
		sensorsMatch[sensorType] = (long)(inVals[0] / TIME_STAMP);
		sensorsCnt[sensorType] = 0;
	}
	else
	{
		sensorsMatch[sensorType] = 1000000000L;
		sensorsCnt[sensorType] = 0;
	}

	// Read value in the main loop.
	sensorsReadOnce[sensorType] = 1;

	return RES_OK;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Get Engduino's sensor measurements
* \param engPackage			Pointer to an Engduino package structure
* \param inNrVals			Number of input values
* \param inVals				Array of input values
* \return					On success RES_OK, otherwise error code represents
*							particular error.
*
* This function will sends button state or enables button interrupt, which
* will immediately sends back any change in button state. Both edges can be
* configured as an interrupt source.
*
* Input value examples:
* [0 ] -> Read once
* [-1] -> Disable button interrupt
* [1 ] -> Sends package on button press 
* [2 ] -> Sends package on button release 
* [3 ] -> Sends package on change of both button states 
*
*/
int EngduinoProtocolClass::getButton(struct EngduinoPackage *engPackage, byte inNrVals, long *inVals)
{
	if(inNrVals > 1) return RES_ERR_PACKAGE_NR_VALS;
	if(inNrVals == 1 && inVals[0] != 0)
	{
		switch(inVals[0])
		{
			case -1:
				buttonCatchEnable[BUTTON_PRESSED] = false;
				buttonCatchEnable[BUTTON_RELEASED] = false;
				break;
			case 1:
				buttonCatchEnable[BUTTON_PRESSED] = true;
				break;
			case 2:
				buttonCatchEnable[BUTTON_RELEASED] = true;
				break;
			case 3:
				buttonCatchEnable[BUTTON_PRESSED] = true;
				buttonCatchEnable[BUTTON_RELEASED] = true;
				break;
			default:
				return RES_ERR_PACKAGE_VAL;
		}
		return RES_OK;
	}

	// Read once.
	(engduinoButton->isPressed()) ? vals[0] = 1 : vals[0] = 0;
	sendPackage(engPackage, 1, COM_GET_BUTTON, vals);
	return RES_OK;
}


/*---------------------------------------------------------------------------*/
/**
* \brief Internal function for read Thermistor values.
* \param outVals		Pointer to an return value of temperature
*
* This function will reads and returns temperature measurement from Engduino's 
* thermistor sensor. Return value is in unit mili °C.
*
*/
void EngduinoProtocolClass::_readThermistor(long *outVals)
{
	// Convert to mili °C.
	outVals[0] = (long)(engduinoThermistor->temperature()*1000.0);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function for read Accelerometer values.
* \param outVals		Pointer to an return value of accelerations
*
* This function will reads and returns [X, Y, Z] acceleration measurements 
* from Engduino's accelerometer sensor. Return values are in unit mili G.
*
*/
void EngduinoProtocolClass::_readAccelerometer(long *outVals)
{
	float xyz[3];
	engduinoAccelerometer->xyz(xyz);

	// Convert to mili G.
	for(byte i=0; i<3; i++) outVals[i] = (long)(xyz[i]*1000.0); 
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function for read Magnetometer values.
* \param outVals		Pointer to an return value of magnetic field
*
* This function will reads and returns [X, Y, Z] magnetic field measurements 
* from Engduino's magnetometer sensor. Return values are in range from 
* -20000 to +20000.
*
*/
void EngduinoProtocolClass::_readMagnetometer(long *outVals)
{
	float xyz[3];
	engduinoMagnetometer->xyz(xyz);

	//The values range from -20000 to +20000.
	for(byte i=0; i<3; i++) outVals[i] = (long)(xyz[i]);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function for read Light value.
* \param outVals		Pointer to an return value from light
*
* This function will reads and returns light measurement from Engduino's 
* light sensor. Return values is in range from 0 to 1023.
*
*/
void EngduinoProtocolClass::_readLight(long *outVals)
{
	// A 10 bit value - so in a range 0-1023.
	outVals[0] = (long)(engduinoLight->lightLevel()); 
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function for read all on-board sensors values.
* \param outVals		Pointer to an return value from all sensors
*
* This function will reads and returns measurements from all sensors 
* at once in order [1x temp, 3x acc, 3x mag, 1x light].
*
*/
void EngduinoProtocolClass::_readSensors(long *outVals)
{
	// Read all sensors.
	int idx = 0;
	_readThermistor(&outVals[idx]); idx = idx + 1;
	_readAccelerometer(&outVals[idx]); idx = idx + 3;
	_readMagnetometer(&outVals[idx]); idx = idx + 3;
	_readLight(&outVals[idx]); idx = idx + 1;
}

/*---------------------------------------------------------------------------*/
/**
* \brief The main loop of Engduino protocol operating system.
*
* This function needs to be called regularly from the main sketch. If there
* is a waiting task, it will be executed, otherwise function will return
* immediately.
*
*/
void EngduinoProtocolClass::mainLoop()
{
	// Oversampling read
	if(overSamplingRead == 1)
	{
		for(byte i = 0; i < NR_SENSORS; i++)
		{
			if(sensorsMatch[i] > 0)
			{
				switch(i)
				{
					case SENSOR_TEMP:
						PRINTLN("OS_SENSOR_TEMP ");
						_readThermistor(vals);
						sensorsSum[SENSOR_TEMP_BUF] += vals[0];
						sensorsSamples[SENSOR_TEMP]++;
						break;
					case SENSOR_ACC:
						PRINTLN("OS_SENSOR_ACC");
						_readAccelerometer(vals);
						for(byte j = 0; j < 3; j++) sensorsSum[SENSOR_ACC_X_BUF + j] += vals[j];
						sensorsSamples[SENSOR_ACC]++;
						break;
					case SENSOR_MAG:
						PRINTLN("OS_SENSOR_MAG");
						_readMagnetometer(vals);
						for(byte j = 0; j < 3; j++) sensorsSum[SENSOR_MAG_X_BUF + j] += vals[j];
						sensorsSamples[SENSOR_MAG]++;
						break;
					case SENSOR_LIGHT:
						PRINTLN("OS_SENSOR_LIGHT");
						_readLight(vals);
						sensorsSum[SENSOR_LIGHT_BUF] += vals[0];
						sensorsSamples[SENSOR_LIGHT]++;
						break;
					case SENSOR_ALL:
						PRINTLN("OS_SENSOR_ALL");
						_readSensors(vals);
						for(byte j = 0; j < NR_SENSORS_BUF; j++) sensorsSum[j] += vals[j];
						sensorsSamples[SENSOR_ALL]++;
						break;
				}
			}
		}
		overSamplingRead = 0;
	}

	for(byte i = 0; i < NR_SENSORS; i++)
	{
		if(((sensorsMatch[i] > 0) && (sensorsCnt[i] >= sensorsMatch[i])) || sensorsReadOnce[i] == 1)
		{
			sensorsCnt[i] = 0;
			switch(i)
			{
				case SENSOR_TEMP:
					PRINTLN("SENSOR_TEMP ");
					if(overSampling == 0 || sensorsSamples[SENSOR_TEMP] == 0)
					{
						_readThermistor(vals);
						// One sample taken.
						vals[1] = 1; 
					}
					else
					{
						vals[0] = sensorsSum[SENSOR_TEMP_BUF] / sensorsSamples[SENSOR_TEMP];
						vals[1] = sensorsSamples[SENSOR_TEMP];
						sensorsSum[SENSOR_TEMP_BUF] = 0;
						sensorsSamples[SENSOR_TEMP] = 0;
					}
					EngduinoProtocol.sendPackage(2, COM_GET_TEMPERATURE, vals);
					break;

				case SENSOR_ACC:
					PRINTLN("SENSOR_ACC");
					if(overSampling == 0 || sensorsSamples[SENSOR_ACC] == 0)
					{
						_readAccelerometer(vals);
						vals[3] = 1;
					}
					else
					{
						for(byte j = 0; j < 3; j++)
						{
							vals[j] = sensorsSum[SENSOR_ACC_X_BUF + j] / sensorsSamples[SENSOR_ACC];
							sensorsSum[SENSOR_ACC_X_BUF + j] = 0;
						}
						vals[3] = sensorsSamples[SENSOR_ACC];
						sensorsSamples[SENSOR_ACC] = 0;
					}
					EngduinoProtocol.sendPackage(4, COM_GET_ACCELEROMETER, vals);
					break;

				case SENSOR_MAG:
					PRINTLN("SENSOR_MAG");
					if(overSampling == 0 || sensorsSamples[SENSOR_MAG] == 0)
					{
						_readMagnetometer(vals);
						vals[3] = 1;
					}
					else
					{
						for(byte j = 0; j < 3; j++)
						{
							vals[j] = sensorsSum[SENSOR_MAG_X_BUF + j] / sensorsSamples[SENSOR_MAG];
							sensorsSum[SENSOR_MAG_X_BUF + j] = 0;
						}
						vals[3] = sensorsSamples[SENSOR_MAG];
						sensorsSamples[SENSOR_MAG] = 0;
					}
					EngduinoProtocol.sendPackage(4, COM_GET_MAGNETOMETER, vals);
					break;

				case SENSOR_LIGHT:
					PRINTLN("SENSOR_LIGHT");
					if(overSampling == 0 || sensorsSamples[SENSOR_LIGHT] == 0)
					{
						_readLight(vals);
						vals[1] = 1;
					}
					else
					{
						vals[0] = sensorsSum[SENSOR_LIGHT_BUF] / sensorsSamples[SENSOR_LIGHT];
						vals[1] = sensorsSamples[SENSOR_LIGHT];
						sensorsSum[SENSOR_LIGHT_BUF] = 0;
						sensorsSamples[SENSOR_LIGHT] = 0;
					}
					EngduinoProtocol.sendPackage(2, COM_GET_LIGHT, vals);
					break;

				case SENSOR_ALL:
					PRINTLN("SENSOR_ALL");
					if(overSampling == 0 || sensorsSamples[SENSOR_ALL] == 0)
					{
						_readSensors(vals);
						vals[8] = 1;
					}
					else
					{
						for(byte j = 0; j < NR_SENSORS_BUF; j++)
						{
							vals[j] = sensorsSum[j] / sensorsSamples[SENSOR_ALL];
							sensorsSum[j] = 0;
						}
						vals[8] = sensorsSamples[SENSOR_ALL];
						sensorsSamples[SENSOR_ALL] = 0;
					}
					EngduinoProtocol.sendPackage(9, COM_GET_SENSORS, vals);
					break;
			}
			sensorsReadOnce[i] = 0;
		}
	}

	// Detect button press or release.
	if(buttonCatchEnable[BUTTON_PRESSED] && engduinoButton->wasPressed())
	{
		PRINTLN("BUTTON_PRESSED");
		vals[0] = 1;
		sendPackage(1, COM_GET_BUTTON, vals);
	}
	else if(buttonCatchEnable[BUTTON_RELEASED] && engduinoButton->wasReleased())
	{
		PRINTLN("BUTTON_RELEASED");
		vals[0] = 2;
		sendPackage(1, COM_GET_BUTTON, vals);
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief Timer 1 interrupt service routine
*
* This function needs to be called regularly from the main sketch. If there
* is a waiting task, it will be executed, otherwise function will return
* immediately.
*
*/
ISR(TIMER1_COMPA_vect)
{
	EngduinoProtocol.cntt++;
	if((EngduinoProtocol.cntt % 500) == 0) PRINTLN(".");
	for(byte i = 0; i < NR_SENSORS; i++)
	{
		EngduinoProtocol.sensorsCnt[i]++;
	}

	if(EngduinoProtocol.overSampling > 0)
	{
		EngduinoProtocol.overSamplingCnt++;
		if(EngduinoProtocol.overSamplingCnt >= EngduinoProtocol.overSamplingMatch)
		{
			EngduinoProtocol.overSamplingRead = 1;
			EngduinoProtocol.overSamplingCnt = 0;
		}
	}
}

/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */
EngduinoProtocolClass EngduinoProtocol = EngduinoProtocolClass();

/** @} */
