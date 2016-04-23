#include <EngduinoLEDs.h>

// LEDs demo
// The aim of this example is to show some of the methods of
// causing the LEDs to light. The first part of the sketch is where
// red, green and blue LEDs chase anti-clockwise around the Engduino,
// getting gradually faster until they blur into white.
//
// When going sufficiently fast, we display red green and blue on all
// LEDs, followed by a range of colours.
//
int   r=0, g=1, b=2;  // These are the LEDs that will be lit initially
float pause;

void setup()
{
  EngduinoLEDs.begin();

  pause = 100;
  EngduinoLEDs.setLED(r, RED);
  EngduinoLEDs.setLED(g, GREEN);
  EngduinoLEDs.setLED(b, BLUE);  
}

void loop() {
  int tail = r; // LED at the tail 

  // Wait a little - and reduce the time gradually so the rate
  // of change in which LEDs are lit speeds up
  //
  delay(pause);
  pause = 0.998*pause;
    
  // Advance the red, green, and blue LED numbers to the next
  // in sequence
  //
  r = (r+1)%16;
  g = (g+1)%16;
  b = (b+1)%16;
  
  // Switch on the LEDs with the given numbers
  //
  EngduinoLEDs.setLED(r, RED);
  EngduinoLEDs.setLED(g, GREEN);
  EngduinoLEDs.setLED(b, BLUE);

  // Switch off the red LED at the tail
  EngduinoLEDs.setLED(tail, OFF);

  // When we're going fast, stop this and change activities.
  //
  if (pause < 1)
  { 
    colour c[2][16] = { {BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF},
                        {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF} };
    
    // Switch all LEDs to red, green, and blue successively.
    //
    EngduinoLEDs.setAll(RED);
    delay(500);
    EngduinoLEDs.setAll(GREEN);
    delay(500);
    EngduinoLEDs.setAll(BLUE);
    delay(500);
    
    // Create a chain of different colours, then move them circularly, first
    // slowly and then much faster. There are two such chains given by the
    // colour array above.
    //
    for (int d=500; d>0; d-=400) {
      for (int i = 0; i < 2; i++) {
        long start = millis();
        // For a period of 5 seconds, display and move the LEDs
        while (millis() < start+5000) {
          // Display the selected LEDs
          EngduinoLEDs.setLEDs(c[i]);
          
          // Now move the colours along circularly
          colour temp = c[i][15];
          for (int j = 15; j > 0; j--)
            c[i][j] = c[i][j-1];
          c[i][0] = temp;

          // And pause slightly
          delay(d);
        }
      }
    }
    
    // And start from the beginning again
    //
    r=0; g=1; b=2;
    pause = 100;    
    EngduinoLEDs.setAll(OFF);
    EngduinoLEDs.setLED(r, RED);
    EngduinoLEDs.setLED(g, GREEN);
    EngduinoLEDs.setLED(b, BLUE);    
  }
}

