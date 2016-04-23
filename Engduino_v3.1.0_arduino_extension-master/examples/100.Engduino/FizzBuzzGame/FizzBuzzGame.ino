#include <EngduinoLEDs.h>
#include <EngduinoButton.h>

//
// This is a game like fizz-buzz where you have to do mental
// arithmetic, keeping a running total of the count of LEDs
// and pushing the button when that count is a multiple of
// three or five
//

// Maximum number of LEDS to light at a time
#define NLEDS 5

// State
//
// Minimum time the LED is on or off for - there is a limit to what
// the human can see and how fast they can respond.
//
long minontime  = 2000;
long minofftime = 1500;
long     OKTime = minontime + minofftime;

// Used in giving people a reward for doing well
//
colour colours[16] = {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, RED, GREEN};

// This is our total count
//
long sum = 0;

void setup()
{
  EngduinoLEDs.begin();
  EngduinoButton.begin();  
  randomSeed(analogRead(0));    // Seed analog input 0, which is unconnected
}

// Called when the user made a mistake. Flashes all the leds
// red a couple of times as a punishment, then gives a run of
// colurful LEDs as a reward for how well the user did.
// 
void oops(int success) {
  colour c[16] = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};

  // Punishment - flash LEDs red
  //
  for (int i = 0; i < 2; i++) {
    EngduinoLEDs.setAll(RED);
    delay(500);
    EngduinoLEDs.setAll(OFF);
    delay(500);    
  }

  // Reward - light up a colurful string of LEDs
  //
  for (int i = 0; i < success; i++) {
    c[i] = colours[i];
    EngduinoLEDs.setLEDs(c);
    delay(500);
  }
  EngduinoLEDs.setAll(OFF);
  delay(500);    
  EngduinoLEDs.setLEDs(c);
  delay(500);
  EngduinoLEDs.setAll(OFF);
  delay(500);    
  EngduinoLEDs.setLEDs(c);
  delay(500);
  EngduinoLEDs.setAll(OFF);
  
  EngduinoButton.reset();  // Clear the wasPressed state  
}


void loop() {
  uint8_t r[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t g[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t b[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t leds[NLEDS] = {0, 0, 0, 0, 0};
  uint8_t n = 0;

  while (true) {    
    // Clear the rgb values from the previous iteration
    //
    for (int i = 0; i < n; i++) {
      r[leds[i]] = 0;
      g[leds[i]] = 0;
      b[leds[i]] = 0;
    }
    
    // Choose a number of LEDs to light
    //
    n = random(1,NLEDS+1); // Pick a number 1-NLEDS;
    sum += n;              // Keep our tunning total
    
    Serial.print("n = " );
    Serial.print(n);
    Serial.print(", sum = " );
    Serial.println(sum);
    
    // Pick the chosen number of different LEDs at random and colour
    // them randomly - make sure we choose different LEDs, so if we
    // choose one we already picked, pick again.
    //
    int i = 0;
    while (i < n) {
      bool found = false;
      leds[i]    = random(0,16);

      for (int j = 0; j < i; j++) {
        if (leds[j] == leds[i]) {
          found = true;
          break;
        }
      }
      
      if (!found) {
        // We chose an LED that we haven't selected before.
        // We can give it any colour but off. The easiest way
        // is to choose a random number from 1 to 7, and
        // use the bits of that number to decide whether to
        // switch on red, green and/or blue channels.
        //
        uint8_t randomColour = random(1, 8);
        r[leds[i]] = ((randomColour & 0x01) == 0) ? 0 : MAX_BRIGHTNESS;
        g[leds[i]] = ((randomColour & 0x02) == 0) ? 0 : MAX_BRIGHTNESS;
        b[leds[i]] = ((randomColour & 0x04) == 0) ? 0 : MAX_BRIGHTNESS;
        
        i++;
      }
    }

    // Choose how long we should have the LED on and off - has a random component.
    //
    long ontime  = minontime  + random(minontime);
    long offtime = minofftime + random(minofftime);
    long  onAt;  // Time the LED went on at - i.e. the time from which the user should react
    long offAt;  // When we switched the LED off
  
    bool mustPush = ((sum%3==0) || (sum%5==0)); // If it's divisible by 3 or 5, the user should push
    bool   pushed = false;

    // Set the chosen LEDs to the chosen colours
    //    
    EngduinoLEDs.setLEDs(r, g, b);

    // And let's see if the button is pressed before the LED goes out
    //    
    onAt = millis();
    delay(ontime);
    pushed = EngduinoButton.wasPressed();

    if ((pushed && !mustPush)) {
      // We're definitely not OK if we pushed and we shouldn't have
      // We might still have a little time left to push if we should
      oops(constrain(round(sum/16), 1, 16));

      // Reset
      minontime = 500;
      sum = 0;      
      continue;                  // We might still have a little time left to push
    }

    
    // Now turn the LED out and wait a while before showing the next
    // We need to continue to check to see if the button was pressed - and
    // if so, we need to test whether it was pressed in time.    
    EngduinoLEDs.setAll(OFF);
    
    offAt = millis();
    while (millis() - offAt < offtime) {
      if (EngduinoButton.wasPressed()) {
        if (mustPush)
          pushed |= ((millis() - onAt) < OKTime);    // If we're too late, we're too late.
        else {
          pushed = true;
          break;
        }
      }
    }

    if ( !((pushed && mustPush) || (!pushed && !mustPush)) ) {
      oops(constrain(round(sum/16), 1, 16));

      // Reset
      minontime = 500;
      sum = 0;      
      continue;                 // We might still have a little time left to push
    }
    
    if (pushed)                 // Reduce the on time gradually: more if the button was
      minontime *= 0.85;        // successfully pushed than successfully not pushed.
    else
      minontime *= 0.9;
  }
}

