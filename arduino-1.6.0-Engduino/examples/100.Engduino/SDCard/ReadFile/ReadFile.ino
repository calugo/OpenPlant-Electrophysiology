#include <EngduinoSD.h>
#include <EngduinoButton.h>

#include <SPI.h>  //Comment this line if you already have include #include <EngduinoLEDs.h>
// Read content in file from SD Card
//
// In this sketch, content of the file stored on SD Card 
// is read and send to the serial terminal.
//

void setup()
{
  EngduinoButton.begin();
  // Press button to start.
  EngduinoButton.waitUntilPressed();
  Serial.begin(9600);

  // See if the card is present and can be initialized:
  if(!EngduinoSD.begin())  
  {
    Serial.println("SD Card failed, or not present!");
    Serial.println("Sketch will terminates here...");
    while(1){;}; // Do not do anything more
  }
  Serial.println("SD Card initialized.");
}

void loop() 
{  
  // Open the file for reading:
  EngduinoSD.open("logger.txt", FILE_READ);
  
  // Read from the file until there's nothing else in it:
  while(EngduinoSD.available()) 
  {
    Serial.write(EngduinoSD.read());
    delay(1);
  }
  
  // Close the file:
  EngduinoSD.close();

  // Press button to read the file again.
  EngduinoButton.waitUntilPressed();
  
  // Add empty line.
  Serial.println("\n"); 
}
