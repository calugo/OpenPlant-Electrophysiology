#include <EngduinoLEDs.h>

// Mood lighting - the LEDs gradually change colour from one
// random colour to another, in an amount of time that is random
// but bounded.
//
// This demonstrates a slowly moving change in the rgb value
// given to the LEDs using an accumulator which ensures that
// each channel changes at an even rate given a time period in
// which to change.
//

uint8_t r, g, b;  // Initial value in rgb space

void setup()
{
  EngduinoLEDs.begin();
  Serial.begin(115200);
  randomSeed(analogRead(0));    // Seed analog input 0, which is unconnected
  
  r = random(1, 16);
  g = random(1, 16);
  b = random(1, 16);
  EngduinoLEDs.setAll(r, g, b);
}

void loop()
{ 
  
  uint8_t new_r,   new_g,   new_b;
  uint8_t dir_r,   dir_g,   dir_b;
  int     delta_r, delta_g, delta_b;
  int     accum_r, accum_g, accum_b;
  
  // Choose the number of steps over which to change colour
  // We delay by 10ms each timestep, so this will make the chosen
  // change a random value  between 10 and 25 seconds
  //
  int n_steps = random(1000, 2500);

  // Choose the next point in RGB space at random
  // Don't choose off.
  //
  do {
    new_r = random(0, 16);
    new_g = random(0, 16);
    new_b = random(0, 16);
  } while (new_r==0 && new_g==0 && new_b==0);

  
  // Calculate direction of change - are we aiming to stay the
  // same, or go up or down in value?
  //
  dir_r = (new_r == r) ? 0 : ((new_r > r) ? 1 : -1);
  dir_g = (new_g == g) ? 0 : ((new_g > g) ? 1 : -1);
  dir_b = (new_b == b) ? 0 : ((new_b > b) ? 1 : -1);

  // What is the absolute change we must achieve in value for each channel
  //
  delta_r = abs(new_r - r);
  delta_g = abs(new_g - g);
  delta_b = abs(new_b - b);
  
  // Zero the accumulators
  //
  accum_r = accum_g = accum_b = 0;
  
  // For each time step, accumulate the change in each channel, and see if
  // it's bigger than n_steps. This is the equivalent of doing a division
  // of delta by n_steps to work out the change per timestep and seeing when
  // the accumulated change is greater than one unit. Division is slow on the
  // Arduino, so we do it this way instead.
  //
  for (int s = 1; s <= n_steps; s++) {
    accum_r += delta_r;
    accum_g += delta_g;
    accum_b += delta_b;

    if (accum_r >= n_steps) {
      r += dir_r;
      accum_r -= n_steps;
    }
    
    if (accum_g >= n_steps) {
      g += dir_g;
      accum_g -= n_steps;
    }
    
    if (accum_b >= n_steps) {
      b += dir_b;
      accum_b -= n_steps;
    }
    
    // Set the colour of the LEDs 
    //
    EngduinoLEDs.setAll(r, g, b);
    
    // And wait for a short while
    //
    delay(10);
  }
}
