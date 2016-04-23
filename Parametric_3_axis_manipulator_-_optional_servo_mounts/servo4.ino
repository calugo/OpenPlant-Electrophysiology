#include <Servo.h>;


char val; // Data received from the serial port
int ledPin = 13; // Set the pin to digital I/O 13

int pushD2 = 2; // extra button
int pushD3 = 3; // not used button
int pushD6 = 6; // not used button
int pushD4 = 4; // up button
int pushD5 = 5; // down button

int JoyX = 0;
int JoyY = 1;


int pushD2val = 0; // high = not pressed
int pushD3val = 0; // high = not pressed
int pushD4val = 0; // high = not pressed
int pushD5val = 0; // high = not pressed
int pushD6val = 0; // high = not pressed

int JoyXval = 0;
int JoyYval = 0;

int Movespeed = 50;
float Joymovespeed_scale = 512/Movespeed;
int Calibration = 0;
int Calib_increment = 5;

Servo XServo;
Servo YServo;
Servo ZServo;
int Xzero_angle = 1450;
int Xturn_speed = 0;
int Yzero_angle = 1490;
int Yturn_speed = 0;
int Zzero_angle = 1490;
int Zturn_speed = 0;

void setup(){

  XServo.attach(11);
  YServo.attach(10);
  ZServo.attach(9);
  XServo.write(Xzero_angle);
  YServo.write(Yzero_angle);
  ZServo.write(Zzero_angle);

  pinMode(pushD2, INPUT);
  pinMode(pushD3, INPUT);
  pinMode(pushD4, INPUT); // Z up
  pinMode(pushD5, INPUT); // Z down
  pinMode(pushD6, INPUT);  

  pinMode(JoyX, INPUT);
  pinMode(JoyY, INPUT);  

  pinMode(ledPin, OUTPUT); // Set pin as OUTPUT
  Serial.begin(9600); // Start serial communication at 9600 bps

}

void loop(){

  digitalWrite(ledPin, LOW);
  ZServo.writeMicroseconds(Zzero_angle);


  // Joystick XY
  JoyXval = analogRead(JoyX);
  JoyYval = analogRead(JoyY);  
  if (Calibration == 0) {
    XServo.writeMicroseconds(Xzero_angle+(JoyXval-511)/Joymovespeed_scale); 
    YServo.writeMicroseconds(Yzero_angle+(JoyYval-511)/Joymovespeed_scale); 
  }
  else
  {
    if (JoyXval>511+50)
    {
      Xzero_angle+=Calib_increment;      
      Calibration = 0;
    }
    if (JoyXval<511-50)
    {
      Xzero_angle-=Calib_increment;      
      Calibration = 0;
    }
    if (JoyYval>511+50)
    {
      Yzero_angle+=Calib_increment;      
      Calibration = 0;
    }
    if (JoyYval<511-50)
    {
      Yzero_angle-=Calib_increment;      
      Calibration = 0;
    }

  }


  // Pushbuttons
  pushD2val = digitalRead(pushD2);
  pushD3val = digitalRead(pushD3);
  pushD4val = digitalRead(pushD4); // Z up
  pushD5val = digitalRead(pushD5); // Z down
  pushD6val = digitalRead(pushD6);  

  if (Calibration == 0) {
    if (pushD4val == 0) // Z up
    {
      ZServo.writeMicroseconds(Zzero_angle+Movespeed); 
      digitalWrite(ledPin, HIGH);
    }


    if (pushD5val == 0) // Z down
    {
      ZServo.writeMicroseconds(Zzero_angle-Movespeed); 
      digitalWrite(ledPin, HIGH);
    }
  }
  else
  {
    if (pushD4val == 0) // Z up calib
    {
      Zzero_angle+= Calib_increment;
      Calibration = 0;
      digitalWrite(ledPin, HIGH);
    }


    if (pushD5val == 0) // Z down calib
    {
      Zzero_angle-= Calib_increment;
      Calibration = 0;

      digitalWrite(ledPin, HIGH);
    }
  }


  if (pushD6val == 0) { // Calibration mode
    digitalWrite(ledPin, HIGH);
    Calibration = 1; 
  } 

  if (pushD3val == 0) { // Calibration mode
    digitalWrite(ledPin, HIGH);
    Movespeed = 200;
    Joymovespeed_scale = 512/Movespeed;
  }
  else{
    Movespeed = 50;
    Joymovespeed_scale = 512/Movespeed;
  } 





  if (Serial.available()) 
  { // If data is available to read,
    val = Serial.read(); // read it and store it in val
  }
  if (val == '0') 
  {
    digitalWrite(ledPin, LOW); // turn the LED off

    XServo.writeMicroseconds(Xzero_angle);
    YServo.writeMicroseconds(Yzero_angle);
    ZServo.writeMicroseconds(Zzero_angle);

  }


  if (val == '1') {
    if (Calibration == 0) {
      digitalWrite(ledPin, HIGH); 
      XServo.writeMicroseconds(Xzero_angle+Movespeed); 

    }
    if (Calibration == 1) { 
      Xzero_angle+=Calib_increment;  
      Serial.write(Xzero_angle);
      delay(500);
      Calibration = 0; 
    }
  } 
  if (val == '2') {
    if (Calibration == 0) {
      digitalWrite(ledPin, HIGH); 
      XServo.writeMicroseconds(Xzero_angle-Movespeed); 

    }
    if (Calibration == 1) { 
      Xzero_angle-=Calib_increment;  
      Serial.write(Xzero_angle);
      delay(500);
      Calibration = 0; 
    }
  } 

  if (val == '3') {
    if (Calibration == 0) {
      digitalWrite(ledPin, HIGH); 
      YServo.writeMicroseconds(Yzero_angle+Movespeed); 

    }
    if (Calibration == 1) { 
      Yzero_angle+=Calib_increment;  
      Serial.write(Yzero_angle);
      Calibration = 0; 
    }
  } 
  if (val == '4') {
    if (Calibration == 0) {
      digitalWrite(ledPin, HIGH); 
      YServo.writeMicroseconds(Yzero_angle-Movespeed); 

    }
    if (Calibration == 1) { 
      Yzero_angle-=Calib_increment;  
      Serial.write(Yzero_angle);
      Calibration = 0; 
    }
  } 
  if (val == '5') {
    if (Calibration == 0) {
      digitalWrite(ledPin, HIGH); 
      ZServo.writeMicroseconds(Zzero_angle+Movespeed); 

    }
    if (Calibration == 1) { 
      Zzero_angle+=Calib_increment;  
      Serial.write(Zzero_angle);
      Calibration = 0; 
    }
  } 
  if (val == '6') {
    if (Calibration == 0) {
      digitalWrite(ledPin, HIGH); 
      ZServo.writeMicroseconds(Zzero_angle-Movespeed); 

    }
    if (Calibration == 1) { 
      Zzero_angle-=Calib_increment;  
      Serial.write(Zzero_angle);
      Calibration = 0; 
    }
  } 


  /// SPEED
  if (val == '7') { 
    Movespeed = 50; 
    Joymovespeed_scale = 512/Movespeed;
    Calibration = 0; 
  } 
  if (val == '8') { 
    Movespeed = 200; 
    Joymovespeed_scale = 512/Movespeed;
    Calibration = 0; 
  } 
  /// CALIBRATION MODE
  if (val == '9') { 
    Calibration = 1; 
  } 


  delay(10); 
}













