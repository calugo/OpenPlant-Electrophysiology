#include <EngduinoSD.h>
#include <EngduinoThermistor.h>
#include <EngduinoButton.h>

#include <SPI.h>  //Comment this line if you already have include #include <EngduinoLEDs.h>
// Data logger fast
//
// In this sketch, temperature values is saved locally and on 
// button pressed saved the SD Card. SD Card is initialized 
// in manual open-close mode, which do not open and close
// file before and after write. Reason for that is because 
// we open the file, save multiple values in it and close it
// after that.
//
// NOTE: Do not forget to close the file after writing complete!
//
// The sketch illustrates the use of both SD Card and thermistor.
//

#define BUFF_SIZE 100

int temp[BUFF_SIZE];
int index = 0;

void setup()
{
  // See if the card is present and can be initialized:
  // Card is initializeed in manual open-close mode.
  if(!EngduinoSD.begin())  
  {
    Serial.println("SD Card failed, or not present");
    while(1){;}; // don't do anything more
  }
  Serial.println("SD Card initialized.");
  
  // Initialize thermistor:
  EngduinoThermistor.begin();

  // Initialize button:
  EngduinoButton.begin();
}

void loop() 
{  
  // Make a string for assembling the data to log:
  String dataString = "Temp is: ";
  
  // Get the temperature:
  float t = EngduinoThermistor.temperature();

  // Add new temperature value to the buffer:
  temp[(index++)%BUFF_SIZE] = int(t);

  if(EngduinoButton.isPressed())
  {
    // Open file:
    EngduinoSD.open("fastWr.txt", FILE_WRITE);
    for(int i = 0; i < BUFF_SIZE; i++)
    {
      dataString = "Temp is: ";
      // Append value to the string:
      dataString += String(temp[i]);
      // Wite on SD Card
      EngduinoSD.writeln(dataString);
    }
    
    // Close file:
    EngduinoSD.close();
    delay(1000); // Delay one second
  }
  
  delay(50); // Delay 50 miliseconds
}

