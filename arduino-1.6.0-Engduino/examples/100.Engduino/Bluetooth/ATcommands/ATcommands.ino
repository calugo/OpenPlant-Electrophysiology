#include <SoftwareSerial.h>
#include <EngduinoButton.h>

// AT Commands demo
// This example shows how to use AT commands with Engduino
// extension Bluetooth module based on HC-05. The AT commands
// are used to change internal parameters of the BT module, like 
// name, role (master/slave), baudrate speed, etc.
//
// IMPORTANT: To enable AT commands KEY and RST jumpers on the 
// bottom side of BT extension board needs to be connected.
//
// This example will check for board version and change the default
// name from "HC-05" to the "EBT-123". All communication between 
// Engduino and BT extension board is forward to the PC serial 
// terminal.
//
// Engduino team: support@engduino.org
// Engduino site: http://www.engduino.org/

// Define I/O pin numbers.
int gLedPin = 13;
int gKeyPin = 12;
int gResetPin = 6;

void setup() 
{
  // Initialize I/Os
  pinMode(gLedPin, OUTPUT);
  pinMode(gResetPin, OUTPUT);
  pinMode(gKeyPin, OUTPUT);
  digitalWrite(gResetPin, LOW);
  digitalWrite(gKeyPin, LOW);
  
  EngduinoButton.begin();

  // Press button to start.
  EngduinoButton.waitUntilPressed();

  Serial.begin(9600);
  Serial.println("AT Commands demo"); 
  
  BtPrintVersion(); 
  BtSetModuleName("EBT-123");
  BtGetModuleName();
}

void loop()
{
  if (Serial1.available())
  {
    // Forward everything to the PC terminal.
    char tmpChar = Serial1.read();
    Serial.print(tmpChar);
  }
}

// Reset the BT module over the reset pin.
void BtReset(void)
{
  Serial.println("BtReset");    
  Serial1.flush();
  delay(200);
  
  // Using the reset pin
  digitalWrite(gResetPin, LOW);
  delay(2000);
  digitalWrite(gResetPin, HIGH);
  delay(500);
}

// Send AT command.
void BtSetCmdMode(int i_cmdMode = 1)
{
  Serial.print("BtSetCmdMode ");    
  Serial.println(i_cmdMode);    
  digitalWrite(gKeyPin, (1 == i_cmdMode) ? HIGH : LOW);
  BtReset();
  
  Serial1.begin((1 == i_cmdMode) ? 38400 : 57600);
}

// Print version of the BT module.
void BtPrintVersion()
{
  Serial1.begin(38400);
  BtSetCmdMode(1);
  Serial1.println("AT");
  Serial.println("AT");
  delay(1000);
  Serial1.println("AT+VERSION?");
  Serial.println("AT+VERSION?");
  delay(1000);
  BtSetCmdMode(0); 
}

// Set the name of the BT module.
void BtSetModuleName(String name)
{
  Serial1.begin(38400);
  BtSetCmdMode(1);
  Serial1.println("AT");
  Serial.println("AT");
  delay(1000);
  Serial1.print("AT+NAME=");
  Serial1.println(name);
  Serial.print("AT+NAME=");
  Serial.println(name);
  delay(1000);
  BtSetCmdMode(0); 
}

// Get the name of the BT module.
void BtGetModuleName()
{
  Serial1.begin(38400);
  BtSetCmdMode(1);
  Serial1.println("AT");
  Serial.println("AT");
  delay(1000);
  Serial1.println("AT+NAME?");
  Serial.println("AT+NAME?");
  delay(1000);
  BtSetCmdMode(0); 
}