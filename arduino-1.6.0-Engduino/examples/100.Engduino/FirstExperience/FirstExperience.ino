#include <EngduinoLEDs.h>
#include <EngduinoButton.h>
#include <EngduinoThermistor.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoMagnetometer.h>
#include <Wire.h>

//abc
// FirstExperience demo
// This is the FirstExperience code that pre-loaded with some of the Engduino V3
// There are four main modes, which can be changed by pressing the button the number of times
// 1. Running LED - a display of lights
// 2. Temperature LED - Blow on the temperature sensor on the Engduino V3, see the colour change from blue to red
// 3. Level - try to make the Engduino level, when it is level, the four LED lights will show green
// 4. A memory game - you are shown the one of the 4 possible positions of green lights. 
//    Try and repeat the pattern by tilting the Engduino to the 4 positions. The number of lights increases by 1
//    every time you get it right. the maximum number of levels is 16. 
// 5. Reaction game - press the button when you see a blue light. 

// Define the temperatures of blue and red.
int TBLUE = 25;
int TRED = 35;
int TRANGE = 10;

int   r=1, g=2, b=3;  // These are the LEDs that will be lit initially
float pause;

int state = 0;
int buttondelay = 300;
boolean changeState = false;
int numWait = 0;
uint8_t brightness = 8;

uint8_t tempStage = 0;
float lastTemp = 0; 

uint8_t gameStage = 0;
uint8_t gamePos[16];

// State
//
// Minimum time the LED is on or off for - there is a limit to what
// the human can see and how fast they can respond.
//
long minontime  = 500;
long minofftime = 500;
long     OKTime = minontime + minofftime;

// Used in giving people a reward for doing well
//
colour colours[16] = {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, RED, GREEN};

//Variable for pulsesensors
//  VARIABLES
int pulsePin = 2;                 // Pulse Sensor purple wire connected to analog pin 1 (AI_1)
int blinkPin = 13;                // pin to blink led at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

int LEDpos = 0;

// Variables for BT
char incomingByte;  // incoming data
int  LED = 13;      // LED pin

String content = "";
char character;
int shake = 0;

colour c[5][16] = { {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF}, 
                      {GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED}, 
                    {BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN},
                 {YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE},
              {MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, OFF, RED, GREEN, BLUE, YELLOW} };

void setup()
{
  EngduinoLEDs.begin();
  Serial.begin(115200);
  EngduinoButton.begin();
  pause = 100;
  
  randomSeed(analogRead(0));    // Seed the random number generator with analog input 0, which is unconnected on the Engduino
  EngduinoThermistor.begin();
  takeBaseTemp();

  EngduinoAccelerometer.begin();
#ifndef __BOARD_ENGDUINOV1
  EngduinoMagnetometer.begin();
#endif
  //setup for pulse sensors
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
  
  // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   analogReference(EXTERNAL); 
   
 EngduinoLEDs.setAll(OFF);  
 //Opening sequence lights
  for (int i=0; i<16; i++) {
    EngduinoLEDs.setAll(BLUE, i);
  
    // Switch off the red LED at the tail
    delay(50);
  }
  delay(500);
  for (int i=15; i>=0; i--) {
    EngduinoLEDs.setAll(BLUE, i);
  
    // Switch off the red LED at the tail
    delay(100);
  }
  delay(10);
  EngduinoLEDs.setAll(OFF);
  delay(1000);   
}

void loop() {
  
    if (EngduinoButton.wasPressed()) {
      if (changeState == false) {
        EngduinoLEDs.setAll(OFF);
        state = 0;
        EngduinoLEDs.setLED(state, RED, brightness);
        changeState = true;
      }
      else {
        state ++;
        EngduinoLEDs.setLED(state, RED, brightness);
      }
      delay(buttondelay);
      
      Serial.println(state);
    }
    else if (changeState == true){
        numWait ++;
        delay(buttondelay);
        if (numWait>=4) {
          changeState = false;
          EngduinoLEDs.setAll(OFF);
          numWait = 0;
        }
        resetLEDs();
    }
    
    
   // void (*p[5])(void) = {&runningLED, &TemperatureLEDs, &Level, &ReactionGame, &pulsesensor};
    
    if (changeState==false) {
      switch (state) {
        case 0:
          runningLED();
          break;
        case 1:
         //takeBaseTemp();
         TemperatureLEDs();          
          break;
        case 2:
          Level();
          break;   
        case 3:
          MemoryGame();
          break;
        case 4: 
          ReactionGame();  
		break;
        /* If you have a pulse sensor, you can add this one too */
        //case 5:
        //  pulsesensor();
		break;
        default: 
          runningLED();
          break;
      }
    }
}






  



