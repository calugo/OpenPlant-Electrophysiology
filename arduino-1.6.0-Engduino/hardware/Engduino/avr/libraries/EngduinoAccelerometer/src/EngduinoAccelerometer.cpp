/**
* \addtogroup EngduinoAccelerometer
*
* This is the driver code for Accelerometer on the Engduino
* In v1.0 this accelerometer is a freescale MMA8652
*
* @{
*/

/**
* \file 
* 		Engduino accelerometer driver
* \author
* 		Engduino team: support@engduino.org
*/

#include "pins_arduino.h"
#include <Wire.h>
#include "EngduinoAccelerometer.h"

/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
* 
* C++ constructor for this class. Empty.
*/
EngduinoAccelerometerClass::EngduinoAccelerometerClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
*
* This function enables the accelerometer.
* The accelerometer is an I2C device so we initialise the I2C communications,
* make contact with the accelerometer, and check that it's what we think it is.
*
* We set the accelerometer to its 2G range, and don't use a FIFO. Other options
* are possible, but will need to be implemented in this driver.
* 
*/
void EngduinoAccelerometerClass::begin() 
{	uint8_t reg;

	// Join I2C bus as master
	//
	Wire.begin();
	
	// Check to see that the device is there
	//
	Wire.beginTransmission(MMA865x_IIC_ADDRESS);
	int error = Wire.endTransmission();  
	if (error != 0) {
		Serial.println("Error: I2C device not found");
		return;
	}
	
	// Check that it's the device we think it is
	//
	readReg(MMA865x_WHO_AM_I, &reg);
	if (reg != 0x5A)  {
		Serial.println("Error: Not an MMA8652");
		return;
	}
	
	// Set up the parameters we'd like to have
	// +-2g and 50Hz
	//
	standby();		// Stop the accelerometer

	// Set to 2g
	readReg(MMA865x_XYZ_DATA_CFG, &reg);
	reg &= ~FS_MASK;
	reg |= FULL_SCALE_2G;
	writeReg(MMA865x_XYZ_DATA_CFG, &reg);
	
	// And 50Hz data rate
	readReg(MMA865x_CTRL_REG1, &reg);
	reg &= ~DR_MASK;
	reg |= DATA_RATE_50HZ;
	writeReg(MMA865x_CTRL_REG1, &reg);
	
	activate();		// And start it again
}

/*---------------------------------------------------------------------------*/
/**
* \brief end function - switch off the button
*
* Send the accelerometer to sleep
*
*/
void EngduinoAccelerometerClass::end() 
{
	standby();		// Stop the accelerometer
}


/*---------------------------------------------------------------------------*/
/**
* \brief Read the xyz values in g from the accelerometer
* \param buf A buffer of floats to put the accelerometer values in g into
*
* Get the instantaneous accelerometer values for the xyz axes. Convert the
* 12 bit digital value to a float value that is expressed in g. 
*
*/
void EngduinoAccelerometerClass::xyz(float buf[3])
{	uint8_t reg[6];
	
	// Check to see if XYZ data is ready to read
	readReg(MMA865x_STATUS_00, reg);

	// Wait on data ready
	while (reg[0] & ZYXDR_MASK == 0)  {
		readReg(MMA865x_STATUS_00, reg);
	}
		
	// Do a multiple read starting with X_MSB
	// because this is the first numbered
	// Read all the MSBs and LSBs = 6 registers
	//
	readReg(MMA865x_OUT_X_MSB, reg, 6);
		
	// Now convert the value to g.
	// At 2g full scale, 1g = 1024 counts
	// The most significant 8 bits are stored
	// in the MSB register, the other 4 bits
	// are in the top of the LSB register, making
	// the number a factor of 16 too big. The
	// complete 12 bit number is stored as
	// 2's complement
	// 
	for (int i = 0; i < 3; i++) {
		float f = (reg[2*i] << 8) + reg[2*i+1];
		f = f / (16 * 1024);	// Put the number into the right range
		buf[i] = f;
	}
		
	// TODO: Could take the cailbration (offset)
	// registers into account
	//
}

/*---------------------------------------------------------------------------*/
/**
* \brief Read the raw xyz values from the accelerometer
* \param buf A buffer of uint16s to hold the 12 bit raw accelerometer values
*
* Get the instantaneous accelerometer values for the xyz axes. We return the
* raw values without converting them into g. To convert to g, if we are using
* a 2G scale, then simply divide by 16384
*
*/
void EngduinoAccelerometerClass::xyzRaw(uint16_t buf[3])
{	uint8_t reg[6];
	
	// Check to see if XYZ data is ready to read
	readReg(MMA865x_STATUS_00, reg);
	
	// Wait on data ready
	while (reg[0] & ZYXDR_MASK == 0)  {
		readReg(MMA865x_STATUS_00, reg);
	}
		
	// Do a multiple read starting with X_MSB
	// because this is the first numbered
	// Read all the MSBs and LSBs = 6 registers
	//
	readReg(MMA865x_OUT_X_MSB, reg, 6);
		
	// Just return raw values
	//
	for (int i = 0; i < 3; i++)
		buf[i] = (reg[2*i] << 8) + reg[2*i+1];
		
	// TODO: Could take the cailbration (offset)
	// registers into account
	//
}

/*---------------------------------------------------------------------------*/
/**
* \brief Activate the sensor
*
* Set the Active bit in System Control Register 1
*
*/
void EngduinoAccelerometerClass::activate()
{	uint8_t reg;

	readReg(MMA865x_CTRL_REG1, 	&reg);
	reg |= ACTIVE_MASK;
	writeReg(MMA865x_CTRL_REG1, &reg);
}


/*---------------------------------------------------------------------------*/
/**
* \brief Put sensor into standby mode
*
* Clear the Active bit in System Control Register 1
*
*/
void EngduinoAccelerometerClass::standby()
{	uint8_t reg;

	readReg(MMA865x_CTRL_REG1, 	&reg);
	reg &= ~ACTIVE_MASK;
	writeReg(MMA865x_CTRL_REG1, &reg);
}


/*---------------------------------------------------------------------------*/
/**
* \brief Internal routine to write accelerometer registers
* \param firstReg The first register to write
* \param buf      The buffer containing things to write to the registers
* \param nRegs	  Optional: The number of registers to write - defaults to 1 
*
* Do a multiple register write of the MMA8652 over the I2C bus - where the
* number of registers written might only be one (as it is if the optional
* nRegs parameter is omitted).
*
*/
void EngduinoAccelerometerClass::writeReg(int firstReg, const uint8_t *buf, uint8_t nRegs)
{
	// Write out the address of the first register, then the set of values
	// for that and all subsequent registers. Release the I2C bus when we're done
	//
	Wire.beginTransmission(MMA865x_IIC_ADDRESS);
	Wire.write(firstReg);
	Wire.write(buf, nRegs);
	Wire.endTransmission();
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal routine to read accelerometer registers
* \param firstReg The first register to read
* \param buf      The buffer to which to read from the registers
* \param nRegs	  Optional: The number of registers to read - defaults to 1 
*
* Do a multiple register read of the MMA8652 over the I2C bus - where the
* number of registers read might only be one (as it is if the optional
* nRegs parameter is omitted).
*
*/
void EngduinoAccelerometerClass::readReg(int firstReg, uint8_t *buf, uint8_t nRegs)
{
	// Write out the register we'd like to start with reading
	// Hold the I2C bus since we're going to read values back
	//
	Wire.beginTransmission(MMA865x_IIC_ADDRESS);
	Wire.write(firstReg);
	Wire.endTransmission(false); 

	// Read all the registers we're interested in,
	// but release the I2C bus when we're done.
	// This is determined by the default third
	// parameter to requestFrom
	//
	Wire.requestFrom((uint8_t)MMA865x_IIC_ADDRESS, nRegs);

	int i = 0;
	while (Wire.available() && i < nRegs)
		buf[i++] = Wire.read();
}


/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */ 
EngduinoAccelerometerClass EngduinoAccelerometer = EngduinoAccelerometerClass();

/** @} */
