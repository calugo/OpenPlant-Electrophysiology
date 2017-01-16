//psstep v. 0.1 - C. Lugo and M. Aita for OpenPlant 20170113

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Psx.h>                                          // Includes the Psx Library 
                                                          // Any pins can be used since it is done in software

#define VERSION "0.1"
#define dataPin 10
#define cmndPin 11
#define attPin 12
#define clockPin 13

#define HALFSTEP 8
#define FULLSTEP 4

//STEPPERS
// The AccelStepper constructor expects the "pins" specified to be the ends of each coil respectively.
// First the ends of the Blue/Yellow coil, then the ends of the Pink/Orange coil (Blue,Yellow,Pink,Orange)
// However, 28BYJ connector, ULN2003 board, and our current configuration is that pins are arranged in the proper FIRING order, 
// Blue, Pink, Yellow, Orange.

#define blue 2
#define pink 3
#define yellow 4
#define orange 5

#define bluex 6
#define pinkx 7
#define yellowx 8
#define orangex 9

#define bluexx 15
#define pinkxx 16
#define yellowxx 17
#define orangexx 18

//#define LEDPin 13

bool clockwise = true;
int initialPositionx = 0;
int initialPositiony = 0;
int initialPositionz = 0;
int savedPositionx = 0;
int savedPositiony = 0;
int savedPositionz = 0;
int targetPositionx = initialPositionx;
int targetPositiony = initialPositiony;
int targetPositionz = initialPositionz;
int stepsize=1; 
int lastdata=0;  //used for debouncing - see psxR1 and psxR2

Psx Psx;                                                  // Initializes the library

unsigned int data = 0; // data stores the controller response

AccelStepper stepper(HALFSTEP, blue, yellow, pink, orange);
AccelStepper stepper2(HALFSTEP, bluex, yellowx, pinkx, orangex);
AccelStepper stepper3(HALFSTEP, bluexx, yellowxx, pinkxx, orangexx);

void setup()
{
  pinMode(blue,OUTPUT);  
  pinMode(pink,OUTPUT); 
  pinMode(yellow,OUTPUT); 
  pinMode(orange,OUTPUT);   
  pinMode(bluex,OUTPUT);  
  pinMode(pinkx,OUTPUT); 
  pinMode(yellowx,OUTPUT); 
  pinMode(orangex,OUTPUT); 
  pinMode(bluexx,OUTPUT);  
  pinMode(pinkxx,OUTPUT); 
  pinMode(yellowxx,OUTPUT); 
  pinMode(orangexx,OUTPUT); 

  pinMode(dataPin, INPUT);
  pinMode(cmndPin, INPUT);
  pinMode(attPin, INPUT);
  pinMode(clockPin, INPUT);
  

  Psx.setupPins(dataPin, cmndPin, attPin, clockPin, 10);  // Defines what each pin is used
                                                          // (Data Pin #, Cmnd Pin #, Att Pin #, Clk Pin #, Delay)
                                                          // Delay measures how long the clock remains at each state,
                                                          // measured in microseconds.
                                                          // too small delay may not work (under 5)
 
  //Stepper1
   //Set the initial speed (read the AccelStepper docs on what "speed" means
   stepper.setSpeed(100.0);         
  //Tell it how fast to accelerate
  stepper.setAcceleration(100.0); 
  //Set a maximum speed it should exceed 
  stepper.setMaxSpeed(4000.0);      
  //Tell it to move to the target position
  stepper.moveTo(targetPositionx); 

  //Stepper2
   //Set the initial speed (read the AccelStepper docs on what "speed" means
  stepper2.setSpeed(100.0);         
  //Tell it how fast to accelerate
  stepper2.setAcceleration(100.0); 
  //Set a maximum speed it should exceed 
  stepper2.setMaxSpeed(4000.0);      
  //Tell it to move to the target position
  stepper2.moveTo(targetPositiony);   

  //Stepper3
  //Set the initial speed (read the AccelStepper docs on what "speed" means
  stepper3.setSpeed(100.0);         
  //Tell it how fast to accelerate
  stepper3.setAcceleration(100.0); 
  //Set a maximum speed it should exceed 
  stepper3.setMaxSpeed(4000.0);      
  //Tell it to move to the target position
  stepper3.moveTo(targetPositionz);   
    
  Serial.begin(9600);
  Serial.write("This is psstep version ");
  Serial.println(VERSION);
}

void loop()
{

  data = Psx.read();                                      // Psx.read() initiates the PSX controller and returns
                                                          // the button data
  //Serial.println(data);                                   // Display the returned numeric value
  if (data & psxSqu){
    clockwise=true;  
    Serial.println(data);                            
    Serial.println("squ!");
    targetPositionx=targetPositionx+stepsize;
    stepper.moveTo(targetPositionx);
    lastdata=data;   
  }
  if (data & psxO){
    clockwise=false;     
    Serial.println(data);                         
    Serial.println("circ!");
    targetPositionx=targetPositionx-stepsize; 
    stepper.moveTo(targetPositionx);
    lastdata=data;   
  }

  if (data & psxX){
    clockwise=true;     
    Serial.println(data);                         
    Serial.println("X!");
    targetPositiony=targetPositiony+stepsize;
    stepper2.moveTo(targetPositiony);
    lastdata=data;      
  }
  if (data & psxTri){
    clockwise=false;   
    Serial.println(data);                           
    Serial.println("tri!");
    targetPositiony=targetPositiony-stepsize; 
    stepper2.moveTo(targetPositiony);  
    lastdata=data;      
 
  }

   if (data & psxUp){
    clockwise=true;     
    Serial.println(data);                         
    Serial.println("up!");
    targetPositionz=targetPositionz+stepsize;
    stepper3.moveTo(targetPositionz);
    lastdata=data;           
  }
  
  if (data & psxDown){
    clockwise=false;   
    Serial.println(data);                           
    Serial.println("down!");
    targetPositionz=targetPositionz-stepsize; 
    stepper3.moveTo(targetPositionz); 
    lastdata=data;      
  }

   if (data & psxL1){   // save last position and move all to initial position  
    Serial.println(data);                           
    Serial.println("go home!");
    if(targetPositionx!=initialPositionx | targetPositiony!=initialPositiony |targetPositionz!=initialPositionz)
    { 
    savedPositionx=targetPositionx;
    savedPositiony=targetPositiony;
    savedPositionz=targetPositionz;
    Serial.write("Saved position x: "); 
    Serial.println(savedPositionx);
    Serial.write("Saved position y: "); 
    Serial.println(savedPositiony);
    Serial.write("Saved position z: "); 
    Serial.println(savedPositionz);
    targetPositionx = initialPositionx;
    targetPositiony = initialPositiony;
    targetPositionz = initialPositionz;
    stepper.moveTo(targetPositionx);   
    stepper2.moveTo(targetPositiony); 
    stepper3.moveTo(targetPositionz); 
    }
  }

     if (data & psxL2){   // move all to saved position  
    Serial.println(data);                           
    Serial.println("go back to saved position!");
    Serial.write("Saved position x: "); 
    Serial.println(savedPositionx);
    Serial.write("Saved position y: "); 
    Serial.println(savedPositiony);
    Serial.write("Saved position z: "); 
    Serial.println(savedPositionz);
    targetPositionx=savedPositionx;
    targetPositiony=savedPositiony;
    targetPositionz=savedPositionz;
    stepper.moveTo(targetPositionx);   
    stepper2.moveTo(targetPositiony); 
    stepper3.moveTo(targetPositionz); 
  }

    if (data & psxR1){  //double the movement step
      if (lastdata!=data)
      {
    Serial.println(data);                           
    Serial.println("faster!");
    stepsize=2*stepsize;
    Serial.write("step size: ");
    Serial.println(stepsize);
    lastdata=data;
      }      
  }

    if (data & psxR2){
        if (lastdata!=data)
        {
          Serial.println(data);    
          Serial.println(lastdata);                        
          Serial.println("slower!");
          if (stepsize>1)
            {
            stepsize=stepsize/2;
            }
          Serial.write("step size: ");
          Serial.println(stepsize);
          lastdata=data;      
        }
  }
   stepper.run();
   stepper2.run();
   stepper3.run(); 

 
   
  //delay(20);
}
