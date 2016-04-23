#include <EngduinoThermistor.h>
#include <EngduinoLEDs.h>

// Temperature LEDs
//
// In this sketch, the colour of the LEDs changes on a linear scale
// from blue (cold) to red (hot) via mixtures of the two colours in
// proportion to the temperature. The temperatures that define blue
// and red are given in the #defines below. Temperatures outside the
// range, are capped so those lower than TBLUE are rendered as blue
// and those higher than TRED are red.
//
// The sketch illustrates the use of both thermistor and LEDs.
//

// Define the temperatures of blue and red.
#define TBLUE 25
#define TRED  35
#define TRANGE (TRED - TBLUE)

void setup()
{
  Serial.begin(115200);
  EngduinoLEDs.begin();
  EngduinoThermistor.begin();
}

void loop()
{   uint16_t r, b;
    
    // Get the temperature
    //
    float t = EngduinoThermistor.temperature();
    
    // Print the temperature
    Serial.print("Temp is: ");
    Serial.println(t);
    
    // Choose the next point in RGB space by temperature - this is a 
    // linear scale that maps from pure blue for the coldest to pure
    // red for the hottest, with intermediate temperatures
    // represented by the appropriate mixture of blue and red.
    //
    // We constrain the temperature so that the lowest temperature we
    // represent is TBLUE and the highest is TRED. MAX_BRIGHTNESS is
    // the maximum we can set an LED channel to.
    //
    r = (constrain(t - TBLUE, 0, TRANGE) * MAX_BRIGHTNESS) / TRANGE;
    b = (constrain(TRED - t,  0, TRANGE) * MAX_BRIGHTNESS) / TRANGE;
    
    // Now set the colour of the all the LEDs appropriately.
    EngduinoLEDs.setAll(r, 0, b);
}



