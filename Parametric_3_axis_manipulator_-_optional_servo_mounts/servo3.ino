#include <Servo.h>;


char val; // Data received from the serial port
int ledPin = 13; // Set the pin to digital I/O 13

int Movespeed = 200;
int Calibration = 0;
int Calib_increment = 5;

Servo XServo;
Servo YServo;
Servo ZServo;
int Xzero_angle = 1450;
int Xturn_speed = 0;
int Yzero_angle = 1490;
int Yturn_speed = 0;
int Zzero_angle = 1500;
int Zturn_speed = 0;

void setup(){

  XServo.attach(11);
  YServo.attach(10);
  ZServo.attach(9);
  XServo.write(Xzero_angle);
  YServo.write(Yzero_angle);
  ZServo.write(Zzero_angle);

  pinMode(ledPin, OUTPUT); // Set pin as OUTPUT
  Serial.begin(9600); // Start serial communication at 9600 bps

}

void loop(){



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

    Calibration = 0; 
  } 
  if (val == '8') { 
    Movespeed = 200; 

    Calibration = 0; 
  } 
  /// CALIBRATION MODE
  if (val == '9') { 
    Calibration = 1; 
  } 


  delay(10); 
}









