/**
* \defgroup EngduinoProtocol Driver for Engduino Protocol
* 
* @{
*/

/**
* \file 
* 		Engduino Protocol driver
* \author
* 		Engduino team: support@engduino.org
*/

#ifndef __ENGDUINOPROTOCOL_H__
#define __ENGDUINOPROTOCOL_H__

#include <Arduino.h>
#include <Engduino.h>

#include <EngduinoLEDs.h>
#include <EngduinoThermistor.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoMagnetometer.h>
#include <EngduinoLight.h>
#include <EngduinoButton.h>
#include <EngduinoIR.h>

// Version
#define VERSION		30	// ENGDUINOV3_0_SKETCH_BASIC 

// Communication channel
#define COMMUNICATION_PC_TERMINAL	1	// PC terminal
#define COMMUNICATION_BT_MODULE		2	// Bluetooth module

// Commands definition
#define COM_SET_LEDS			10
#define COM_SET_LED				11

#define COM_SET_DIGITAL_TYPE	20
#define COM_SET_ANALOG_TYPE		21
#define COM_SET_OUTPUT_DIGITAL	22
#define COM_SET_OUTPUT_ANALOG	23

#define COM_SET_IR				40

#define COM_SET_STATUS			90

#define COM_GET_VERSION			100

#define COM_GET_SENSORS			110
#define COM_GET_TEMPERATURE		111
#define COM_GET_ACCELEROMETER	112
#define COM_GET_MAGNETOMETER	113
#define COM_GET_LIGHT			114

#define COM_GET_BUTTON			120

#define	COM_GET_INPUT_DIGITAL	130
#define	COM_GET_INPUT_ANALOG	131

#define	COM_GET_IR				160

#define COM_GET_STATUS			190


// Package definition
#define PACKAGE_TYPE_1		1
#define PACKAGE_TYPE_2		2
#define PACKAGE_TYPE_3		3

#define PACKAGE_PACKAGE_TYPE 		0
#define PACKAGE_PACKAGE_COMMAND_ID 	1

#define PACKAGE_MAX_LENGTH		156
#define PACKAGE_MAXNR_VALUES 	56

#define PACKAGE_START_CHR		'{'
#define PACKAGE_STOP_CHR		'}'
#define PACKAGE_DELIMITER_CHR	';'

// Status key codes
#define STATUS_OVERSAMPLING	0	// Enable internal oversampling

// Error codes definition
#define RES_OK	  0
#define RES_ERR	 -1
#define RES_ERR_PACKAGE_HEADER	-10
#define RES_ERR_PACKAGE_NR_VALS	-11
#define RES_ERR_PACKAGE_VAL		-12
#define RES_ERR_PACKAGE_UNKNOWN_SENSOR_TYPE -13
#define RES_ERR_PACKAGE_UNKNOWN_STATUS_KEY	-14


// General
#define NR_SENSORS	5	  // n+1 (ALL)
#define TIME_STAMP	2.05  // OS tick in [ms]
#define MIN_TIME	20    // Minimal time between sending packages in [ms]

// Structures
struct EngduinoPackage {
    byte packageType;
	byte commandID;
	long packageID;
	byte ack;
};

enum {
	SENSOR_ALL =	0,
	SENSOR_TEMP =	1,
	SENSOR_ACC =	2,
	SENSOR_MAG =	3,
	SENSOR_LIGHT =  4
};

enum {
	SENSOR_TEMP_BUF =	0,
	SENSOR_ACC_X_BUF =	1,
	SENSOR_ACC_Y_BUF =	2,
	SENSOR_ACC_Z_BUF =	3,
	SENSOR_MAG_X_BUF =	4,
	SENSOR_MAG_Y_BUF =	5,
	SENSOR_MAG_Z_BUF =	6,
	SENSOR_LIGHT_BUF =  7
};
#define NR_SENSORS_BUF	8	// Need to update if more sensors added! Value should be equal to elements in enum above.

enum {
	BUTTON_PRESSED =	0,
	BUTTON_RELEASED =	1
};


// Needed to ensure correct linkage between C++ and C linkage of ISR
extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal));

class EngduinoProtocolClass 
{
	protected:
		EngduinoLEDsClass* engduinoLEDs;
		EngduinoThermistorClass* engduinoThermistor; 
		EngduinoAccelerometerClass* engduinoAccelerometer; 
		EngduinoMagnetometerClass* engduinoMagnetometer;
		EngduinoLightClass* engduinoLight;
		EngduinoButtonClass* engduinoButton;
		EngduinoIRClass* engduinoIR;

	private:
		byte	commChannel;
		colour	ledsColor[16];
		uint8_t	ledsBrightness[16];
		uint8_t	ledsRGB[3][16];
		long	vals[16];
		long sensorsCnt[NR_SENSORS];
		long sensorsMatch[NR_SENSORS];
		uint8_t sensorsReadOnce[NR_SENSORS];
		bool buttonCatchEnable[2];
		uint8_t irBuf[IRBUFSZ];
		long cntt;

		long	 sensorsSum[NR_SENSORS_BUF];
		uint16_t sensorsSamples[NR_SENSORS];
		uint16_t overSamplingCnt;
		uint16_t overSamplingMatch;
		uint8_t  overSamplingRead;

		// Status vars
		uint16_t overSampling;

		void _readThermistor(long *outVals);
		void _readAccelerometer(long *outVals);
		void _readMagnetometer(long *outVals);
		void _readLight(long *outVals);
		void _readSensors(long *outVals);

	public:
		EngduinoProtocolClass();
		~EngduinoProtocolClass();
		void begin(EngduinoLEDsClass *eledsc, 
				   EngduinoAccelerometerClass *eaccc,
				   EngduinoThermistorClass *etc,
				   EngduinoMagnetometerClass *emc,
				   EngduinoLightClass *elc,
				   EngduinoButtonClass *ebc,
				   EngduinoIRClass *eirc);
		void end();
		void mainLoop();
		void loadDefaults();
		int parsePackage(String str, byte *outNrVals, long *outVals);
		int parsePackageHeader(byte communicationChannel, byte inNrVals, long *inVals, struct EngduinoPackage *outPackage, byte *outHeaderSize);
		int sendPackage(byte inNrVals, byte inCommandID, long *inVals);
		int sendPackage(struct EngduinoPackage *engPackage, byte inNrVals, byte inCommandID, long *inVals);
		int setLED(struct EngduinoPackage *engPackage, byte nrVals, long *inVals);
		int setLEDs(struct EngduinoPackage *engPackage, byte nrVals, long *inVals);
		int setGetStatus(struct EngduinoPackage *engPackage, byte nrVals, long *inVals, uint8_t setGet);
		int setGetIR(struct EngduinoPackage *engPackage, byte nrVals, long *inVals, uint8_t setGet);
		int getVersion(struct EngduinoPackage *engPackage);
		int getSensor(struct EngduinoPackage *engPackage, int sensorType, byte nrVals, long *inVals); 
		int getButton(struct EngduinoPackage *engPackage, byte nrVals, long *inVals);
		

		/*
		 * The ISR needs access to the private variables, so we declare it
		 * a friend of the class
		 */
		friend void TIMER1_COMPA_vect();
};

extern EngduinoProtocolClass EngduinoProtocol;

#endif

/** @} */
