#include <EngduinoLEDs.h>

// A constant white light as bright as
// it can be made. Useful as a bike front
// light, cupboard light etc.
//

void setup()
{
  EngduinoLEDs.begin(); 
}

void loop()
{ 
  EngduinoLEDs.setAll(WHITE);
}
