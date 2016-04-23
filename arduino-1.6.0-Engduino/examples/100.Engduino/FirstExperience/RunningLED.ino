//create some global variables to replace while loop - give the code handle back to the main loop

int de, seq;
unsigned long startLEDs;
colour initColours[2][16] = { {BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF},
                        {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF} };
colour ledColours[2][16];
uint8_t brightn[16]={brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness,brightness, brightness, brightness};
boolean first = true;

void runningLED () {
  int tail = r; // LED at the tail 


  // FIRST RUNNING SEQUENCE
  // Wait a little - and reduce the time gradually so the rate
  // of change in which LEDs are lit speeds up
  //
  if (pause >= 1) {
    delay(pause);
    pause = 0.98*pause;
      
    // Advance the red, green, and blue LED numbers to the next
    // in sequence
    //
    r = r%16 + 1;
    g = g%16 + 1;
    b = b%16 + 1;
    
    // Switch on the LEDs with the given numbers
    //
    EngduinoLEDs.setLED(r, RED, brightness);
    EngduinoLEDs.setLED(g, GREEN, brightness);
    EngduinoLEDs.setLED(b, BLUE, brightness);
  
    // Switch off the red LED at the tail
    EngduinoLEDs.setLED(tail, OFF);
  }

  
  //SECOND RUNNING SEQUENCE
  // When we're going fast, stop this and change activities.
  // Switch all LEDs to red, green, and blue successively.
  //
  // Create a chain of different colours, then move them circularly, first
  // slowly and then much faster. There are two such chains given by the
  // colour array above.
  //
  if (pause < 1)
  { 
    if (first) {
      changeActivity();
      first = false;
      startLEDs = millis();
    }
    
    // run the light sequence, return to the main loop after every change   
    EngduinoLEDs.setLEDs(ledColours[seq], brightn);         
    // Now move the colours along circularly
    colour temp = ledColours[seq][15];
    for (int j = 15; j > 0; j--)
      ledColours[seq][j] = ledColours[seq][j-1];
    ledColours[seq][0] = temp;

    // And pause slightly
    delay(de);
    
    
    if (millis() >= startLEDs+5000) {
     startLEDs = millis();
     seq++;
     if (seq == 2) {
      seq = 0;
      de-=400;
    } 
    if (de<0) {
         resetLEDs();  // go back to activity 1
         EngduinoLEDs.setAll(OFF);
         EngduinoLEDs.setAll(RED, brightness);
         delay(500);
         EngduinoLEDs.setAll(GREEN, brightness);
         delay(500);
         EngduinoLEDs.setAll(BLUE, brightness);
         delay(500);
         EngduinoLEDs.setAll(OFF);
     }
   }
   Serial.print("seq " );
   Serial.println(seq); 
   Serial.print("de ");
   Serial.println(de); 
     
  }  
  
}

void resetLEDs() {
  pause = 100; 
  r=1; g=2; b=3;  
  first = true;
}

void changeActivity() {
    EngduinoLEDs.setAll(OFF);
    EngduinoLEDs.setAll(RED, brightness);
    delay(500);
    EngduinoLEDs.setAll(GREEN, brightness);
    delay(500);
    EngduinoLEDs.setAll(BLUE, brightness);
    delay(500);
    EngduinoLEDs.setAll(OFF);
    memcpy(&ledColours, &initColours, sizeof ledColours);
    de = 500;
    seq = 0;
}
