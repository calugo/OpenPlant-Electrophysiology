/****************************/
/* The player is shown a sequence of green lights in 4 directions 
   start with Level 1 with 1 position. The user has to tilt the board 
   towards the position shown. If they get it right, they move up a level
   and is shown 1 extra position. The maximum number of position is 16.  */
/****************************/

//00000000
//3      1      
//3      1
//3      1
//22222222

// Called when the user made a mistake. Flashes all the leds
// red a couple of times as a punishment, then gives a run of
// colurful LEDs as a reward for how well the user did.
// 


//#define DEBUG

int lightDelay = 700;
uint8_t brign = 3;  //brightness
// In this game, the user needs to remember the sequence of light and tilt it the same way
uint8_t falseAttempt = 0;    


void MemoryGame() {
  int ti = 0;
  int pos = 0;
  boolean ans;
  //flash stage
  for (ti=0; ti<gameStage; ti++) {
    gamePos[ti]=random(4);
    flashLight(gamePos[ti], false);
    
  }
  
  
  for (ti=0; ti<gameStage; ti++) {
      ans=userTilt(gamePos[ti]);  
      if (ans == false) {
#ifdef DEBUG
        Serial.println("wrong or no answer");
#endif
        falseAttempt++;
        if (falseAttempt >= 3) {  //if they get a stage wrong 3 time, I am afraid they have to start again!
          falseAttempt = 0;
          gameStage = 0;
          mistake();
          flashStage(1);
        }
        break;
      }
    
  }
  //user's turn
 
 
 if (ti == gameStage) {
   if (gameStage == 16) {
     // flash lights
     victory();
     if (lightDelay > 300)
       lightDelay-=100;
     gameStage = 0;
   }
   else {
      gameStage++; 
      falseAttempt = 0;
      delay(500);
   }
 }
 else {
   flashStage(gameStage);
 }
 //complete all stages, flash congratulation lights! and start again

   
}

int userTilt(int playPos)
{

  float xyz[3];
  uint8_t userPos = 10;
  boolean answer = false;
  boolean moved = false;
  float sens = 0.3;    
  
  //sensitivity setting to return to neutral position, to get ready for an input
  // Read the acceleration
  //
  EngduinoAccelerometer.xyz(xyz);
  
  // And light the appropriate LEDs depending on whether we're level
  // or not. The LEDs chosen are on opposite sides of the board.
  //
#ifdef DEBUG
  Serial.print(xyz[0]);
  Serial.print(", ");
  Serial.print(xyz[1]);
  Serial.print(", ");
  Serial.println(xyz[2]);
#endif
  unsigned long posStart = millis();
  //Needs to return to the neutral position every time.
  while (!(xyz[0] < sens && xyz[0] >-sens && xyz[1] < sens & xyz[1] > -sens)) 
  { 
    if (millis()-posStart>=5000) {
      mistake();
      return answer;   //give user 5 seconds to get ready
    }
    EngduinoAccelerometer.xyz(xyz);
  }
  
  posStart = millis();
  while (!moved) {
    if (millis()-posStart>=5000) 
      break;   //give user 5 seconds to get ready
      
    EngduinoAccelerometer.xyz(xyz);
#ifdef DEBUG
    Serial.print(xyz[0]);
    Serial.print(", ");
    Serial.print(xyz[1]);
    Serial.print(", ");
    Serial.println(xyz[2]);
#endif
    // which is the maximum tilt direction?
    float maxTilt = xyz[0]; 

    if (xyz[0] > 0.2) {
      userPos = 3; 
      moved = true;
    }
    if  (xyz[0] < -0.2 && abs(xyz[0]) > maxTilt) {
      userPos = 1;
      moved = true;
      maxTilt = abs(xyz[0]);
    }
    if (xyz[1] > 0.2 && abs(xyz[1]) > maxTilt) {
      userPos = 0;  
      moved = true;
      maxTilt = abs(xyz[1]);
    }
    if (xyz[1] < -0.2 && abs(xyz[1]) > maxTilt){
      userPos = 2;
      moved = true;
      maxTilt = abs(xyz[1]);
    }
  }
#ifdef DEBUG  
  Serial.print("user position: ");
  Serial.println(userPos);
#endif  
  flashLight(userPos, true); 
  if (userPos == playPos) {
     answer = true;
  }
  else {
    mistake();
  }
  
  // Wait 50ms, then loop
  //
  return answer;
}


// In this function, we flash the light corresponding to the position in the game. 
void flashLight(int pos, boolean play) {
  colour Li;
  uint8_t b;
  
  if (play) {
     Li = GREEN;
     b = brign;
  }  
  else {
     Li = BLUE;
     b = brign + 1;
  }
     
  switch (pos) {
    case 0:
      EngduinoLEDs.setLED(6, Li, b);
      EngduinoLEDs.setLED(7, Li, b);
      EngduinoLEDs.setLED(8, Li, b);
      EngduinoLEDs.setLED(9, Li, b);
      break;
    case 1:
      EngduinoLEDs.setLED(1, Li, b);
      EngduinoLEDs.setLED(2, Li, b);
      EngduinoLEDs.setLED(3, Li, b);
      EngduinoLEDs.setLED(4, Li, b);
      EngduinoLEDs.setLED(5, Li, b);
      break;
    case 2:
      EngduinoLEDs.setLED(0, Li, b);
      EngduinoLEDs.setLED(13, Li, b);
      EngduinoLEDs.setLED(14, Li, b);
      EngduinoLEDs.setLED(15, Li, b);
      break;
    case 3:
      EngduinoLEDs.setLED(10, Li, b);
      EngduinoLEDs.setLED(11, Li, b);
      EngduinoLEDs.setLED(12, Li, b);
      break;
  }
  if (play)
    delay(500);
  else
    delay(lightDelay);
    
  EngduinoLEDs.setAll(OFF);
  delay(200);
}

// Light sequence showing the current stage of the game
void flashStage(int s) {
  EngduinoLEDs.setAll(OFF);
  for (int i=0; i<s; i++){
    EngduinoLEDs.setLED(i, RED, brign);
    delay(200);
  }
  delay(1000);
  EngduinoLEDs.setAll(OFF);
}

// Called when the user made a mistake. Flashes all the leds
// red a couple of times as a punishment; 
// 
void mistake() {

  // Punishment - flash LEDs red
  //
  for (int i = 0; i < 2; i++) {
    EngduinoLEDs.setAll(RED, brign);
    delay(500);
    EngduinoLEDs.setAll(OFF);
    delay(500);    
  }
}
void victory() {
  // Create a chain of different colours, then move them circularly, first
    // slowly and then much faster. There are two such chains given by the
    // colour array above.
    //
    colour c[16] = { BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF, BLUE, OFF};
    uint8_t b[16]={brign, brign, brign, brign, brign, brign, brign, brign, brign, brign, brign, brign, brign,brign, brign, brign};
    for (int d=500; d>0; d-=400) {
        long start = millis();
        // For a period of 5 seconds, display and move the LEDs
        while (millis() < start+2000) {
          // Display the selected LEDs
          EngduinoLEDs.setLEDs(c, b);
          // Now move the colours along circularly
          colour temp = c[15];
          for (int j = 15; j > 0; j--)
            c[j] = c[j-1];
          c[0] = temp;
          
          
          // And pause slightly
          delay(d);
        }
   }
}    
  
  
