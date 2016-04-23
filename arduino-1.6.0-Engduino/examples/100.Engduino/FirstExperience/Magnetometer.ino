#ifndef __BOARD_ENGDUINOV1
#include "EngduinoMagnetometer.h"
#include "Wire.h"
#include <EngduinoLEDs.h>


// Magnetometer demo
//
// Print the field strength values
//
float val = 0.0;



void Magnetometer()
{ 
  float magneticField[3];
  // Read magnetic field strength. The values range from -20000
  // to +20000 counts and are based on internal calibration
  // values
  //
  EngduinoMagnetometer.xyz(magneticField);
  
  float x = magneticField[0]+650;
  float y = magneticField[1]-350;
  float z = magneticField[2]-1200;
  Serial.print("Magnetic field strength: x = ");
  Serial.print(x);
  Serial.print(" counts y = ");
  Serial.print(y);
  Serial.print(" counts z = ");
  Serial.print(z);
  Serial.println(" counts");
  
  // Note that this is an uncalibrated temperature
  // of the die itself. Whilst it should be a value
  // in degrees C, the lack of calibration could mean
  // that it's anything.
  //int8_t t = EngduinoMagnetometer.temperature();
  //Serial.print("Temperature: ");
  //Serial.println(t);
  
  delay(100);
  
  float field = sqrt(x*x + y*y + z*z);
  Serial.print("field: ");
  Serial.println(field);
  
  val = val * 0.6 + field*0.4; // filter
  float dir;
  //direction
  if (y>0)
     dir = 90 - atan(x/y)*180/3.14;
  else if (y<0)
     dir = 270 - atan(x/y)*180/3.14; 
  else {
     if (x<0)
        dir = 180;
     if (x>0)
        dir = 0;
  }
 
 Serial.print("dir = ");
 Serial.println(dir); 
  
  int maxVal = val/16;
  EngduinoLEDs.setAll(OFF);
  
  int dlight[] = {3,5,7,8,9,10,11,12,13,14,15,1};
  int dirslide = (int) dir/30;
  
  EngduinoLEDs.setLED(dlight[dirslide], RED);
  EngduinoLEDs.setLED((int)maxVal, RED);
  for(int i= 0; i<maxVal; i++)
  {
    EngduinoLEDs.setLED(i, RED);
  }
  
}
#endif
