#include <EngduinoSD.h>
#include <EngduinoThermistor.h>
#include <EngduinoLEDs.h>
#include <EngduinoButton.h>

//#include <SPI.h>  //Comment this line if you already have include #include <EngduinoLEDs.h>
// Data logger
//
// In this sketch, temperature value is saved on the SD Card every
// second. SD Card is initialized in auto open-close mode, which
// takes care on opening and closing file before and after write.
//
// The sketch illustrates the use of both SD Card and thermistor.
//

// Global variables:
int cnt = 0;
unsigned long time = 0;

void setup()
{
  // See if the card is present and can be initialized:
  // Card is initialized in auto open-close mode.
  // Driver itself will takes care to open and close file
  // before and after write.
  EngduinoButton.begin();
  // Press button to start.
  EngduinoButton.waitUntilPressed();
  Serial.begin(9600);
  EngduinoLEDs.begin();
  if(!EngduinoSD.begin("logger.txt", FILE_WRITE))  
  {
    EngduinoLEDs.setAll(RED);
    Serial.println("SD Card failed, or not present!");
    Serial.println("Sketch will terminates here...");
    while(1){;}; // don't do anything more
  }
  
  Serial.println("SD Card initialized.");
  
  // Add file header.
  String dataString = "count \t secs \t temp";
  EngduinoSD.writeln(dataString);
  
  // Initialize thermistor.
  EngduinoThermistor.begin();
}

void loop() 
{    
  // Get time in milliseconds
  time = millis(); 
  
  // Get the temperature:
  float t = EngduinoThermistor.temperature();
  t = t * 1000.0; // Convert to millidegrees

  // Compose string from multiple values. 
  // Character "\t" represents Tabulator spacer.
  String dataString = String(cnt) + "\t" +
                      String(time) + "\t" + 
                      String(int(t));

  if(EngduinoSD.writeln(dataString))
  {
    Serial.println("Write done!");
    EngduinoLEDs.setAll(GREEN);
    }
  else
  {
    Serial.println("Write error!");
    EngduinoLEDs.setAll(RED);
  }
  
  // Increase sequence number.
  cnt = cnt + 1;
  
  // To obtain more accurate delay of one second, we need to 
  // subtract code execution time (in our case 22 milliseconds)
  // from program delay. To avoid this you can implement your 
  // own interrupt driven timer which will be called regularly 
  // on the specified interval.
  delay(1000 - 22); 
  EngduinoLEDs.setAll(OFF);
}
