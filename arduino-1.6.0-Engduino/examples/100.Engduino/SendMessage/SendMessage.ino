#include <EngduinoLEDs.h>
#include <EngduinoButton.h>
#include <EngduinoIR.h>

// Send message
// This tests the IR communications.
//
// Pressing a button on an Engduino causes a message to be
// sent. The LEDs flash blue when the message ("Hello World")
// is sent. An Engduino receiving the message will flash green
// if it receives the message correctly, and red if not.
// 
// Illustrates use of IR, LEDS and the button
//
long count = 0;

void setup()
{
  EngduinoLEDs.begin();
  EngduinoButton.begin();
  EngduinoIR.begin();  
}

void loop() {
  int     len = 0;
  uint8_t buf[IRBUFSZ];

  // The message to send
  //
  char *msg = "Hello world";

  // Check to see whether the button has been pressed
  // since the last time we checked.
  //
  if (EngduinoButton.wasPressed()) {
    // The button has been pressed so send a message
    // and flash the LEDs blue
    //
    EngduinoIR.send(msg, strlen(msg)+1);

    EngduinoLEDs.setAll(BLUE);
    delay(50);
    EngduinoLEDs.setAll(OFF);
    delay(50);
  }


  // Now check to see if there's a message to read
  // Wait 1ms for a message to start being received,
  // else time out.
  //
  len = EngduinoIR.recv(buf, 1000);
  if (len < 0) {
    // If we timed out, the return value is negative
    // Print a . every 1000 times we time out just so
    // we can tell the system is still alive..
    //
    count++;
    if (count%1000 == 0) {
      Serial.print(".");
      if (count%80000 == 0) {
        count = 0;
        Serial.println("");
      }
    }
  }
  else {
    // We received data. Print it out - first in hex
    //
    Serial.println("");
    Serial.print("Data [");
    Serial.print(len, DEC);
    Serial.print("]: ");  
    for (int i=0; i<len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(", ");
    }
    Serial.println("");

    // And now print it as a string
    //
    if (len > 1) {
      Serial.println((char *)buf);
    }

    // If the data is what we expect, flash the LEDs green
    // and if it is not, flash them red.
    //
    if (!strcmp((char *)buf, "Hello world")) {
      EngduinoLEDs.setAll(GREEN);
      delay(50);
      EngduinoLEDs.setAll(OFF);
      delay(50);    
    }
    else {
      EngduinoLEDs.setAll(RED);
      delay(50);
      EngduinoLEDs.setAll(OFF);
      delay(50);
    }
  }
}



