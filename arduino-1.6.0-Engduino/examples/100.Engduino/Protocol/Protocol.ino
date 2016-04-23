#include <EngduinoProtocol.h>
#include <EngduinoLEDs.h>
#include <EngduinoThermistor.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoMagnetometer.h>
#include <EngduinoLight.h>
#include <EngduinoButton.h>
#include <EngduinoIR.h>
#include <Wire.h>

// Variables
String packageContent;
char chr;
boolean packageReceived;
byte packageLength;
long packageVals[PACKAGE_MAXNR_VALUES]; // int32
byte packageNrVals;
int res;

EngduinoPackage engPackage;

 // Called only once (during start-up).
void setup() 
{
  Serial1.begin(9600); 	// Initialization of BT module
  Serial.begin(9600);	// PC Terminal
  
  EngduinoLEDs.begin();
  EngduinoAccelerometer.begin();
  EngduinoThermistor.begin();
  EngduinoMagnetometer.begin();
  EngduinoLight.begin();
  EngduinoButton.begin();
  EngduinoIR.begin();
  
  // Initialize Engduino communication protocol
  EngduinoProtocol.begin(&EngduinoLEDs, 
                         &EngduinoAccelerometer,
                         &EngduinoThermistor,
                         &EngduinoMagnetometer,
                         &EngduinoLight,
                         &EngduinoButton,
                         &EngduinoIR);
  
  packageContent = "";
  packageReceived = false;
  packageLength = 0;
}

 // Infinity loop
void loop() 
{
  while(Serial.available())
  {
    chr = Serial.read();
    readPackegeByChar(COMMUNICATION_PC_TERMINAL);
  }
  while(Serial1.available())
  {
    chr = Serial1.read();
    readPackegeByChar(COMMUNICATION_BT_MODULE);
  }
  
  EngduinoProtocol.mainLoop(); // call main loop	
}

void readPackegeByChar(byte communicationChannel) 
{
  if(chr == PACKAGE_START_CHR)
  {
    packageContent = "";
    packageReceived = false;
    packageLength = 0;
  }
  else if(chr == PACKAGE_STOP_CHR)
  {
    packageReceived = true;
  }
  else if(packageLength < PACKAGE_MAX_LENGTH) 
  {
    packageContent.concat(chr);
    packageLength++;
  }

  if(packageReceived == true) 
  {
    //Serial.println("Received!");
    res = executeCommand(communicationChannel, packageContent, packageVals, &engPackage);
    packageReceived = false;
  }
}

int executeCommand(byte communicationChannel, String packageContent, long *packageVals, struct EngduinoPackage *engPackage)
{
  byte startIndex = 0;
  int res;
  byte packageNrValsAll;
  byte packageNrVals;
  res = EngduinoProtocol.parsePackage(packageContent, &packageNrValsAll, packageVals);
  if(res != RES_OK) return res;
	
  res = EngduinoProtocol.parsePackageHeader(communicationChannel, packageNrValsAll, packageVals, engPackage, &startIndex);
  if(res != RES_OK) return res;

  packageNrVals = packageNrValsAll - startIndex;

  switch (engPackage->commandID) 
  {
    case COM_SET_LED:
      res = EngduinoProtocol.setLED(engPackage, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_SET_LEDS:
      res = EngduinoProtocol.setLEDs(engPackage, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_SET_IR:
      res = EngduinoProtocol.setGetIR(engPackage, packageNrVals, &packageVals[startIndex], 0);
      break;
    case COM_SET_STATUS:
      res = EngduinoProtocol.setGetStatus(engPackage, packageNrVals, &packageVals[startIndex], 0);
      break;
    case COM_GET_VERSION:
      res = EngduinoProtocol.getVersion(engPackage);
      break;
    case COM_GET_TEMPERATURE:
      res = EngduinoProtocol.getSensor(engPackage, SENSOR_TEMP, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_GET_ACCELEROMETER:
      res = EngduinoProtocol.getSensor(engPackage, SENSOR_ACC, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_GET_MAGNETOMETER:
      res = EngduinoProtocol.getSensor(engPackage, SENSOR_MAG, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_GET_LIGHT:
      res = EngduinoProtocol.getSensor(engPackage, SENSOR_LIGHT, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_GET_SENSORS:
      res = EngduinoProtocol.getSensor(engPackage, SENSOR_ALL, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_GET_BUTTON:
      res = EngduinoProtocol.getButton(engPackage, packageNrVals, &packageVals[startIndex]);
      break;
    case COM_GET_STATUS:
      res = EngduinoProtocol.setGetStatus(engPackage, packageNrVals, &packageVals[startIndex], 1);
      break;
    case COM_GET_IR:
      res = EngduinoProtocol.setGetIR(engPackage, packageNrVals, &packageVals[startIndex], 1);
      break;
    default: 
      // statements
      break;
  }
}