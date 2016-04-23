



void serialEvent(Serial port){
 if (!processingSerial) {
   processingSerial = true;
    while (port.available()>0) {
      // get the new byte:
      char inChar = (char)port.read(); 
      // add it to the inputString:
      if (inChar != '\0')
        inData += inChar;
      // if the incoming character is a newline, set a flag
      // so the main loop can do something about it:
      if (inChar == '\n') {
        stringComplete = true;
        break;
      }
    }
    
  if (stringComplete) {
      processSerialData();
      stringComplete = false;
  inData = "";
 }
 processingSerial = false;
 }
 
  
} 
  
  
 

void processSerialData() {
   inData = trim(inData);                 // cut off white space (carriage return)
   
   switch (inData.charAt(0)) {
     case 'S':          // leading 'S' for sensor data
       inData = inData.substring(1);        // cut off the leading 'S'
       Sensor = int(inData);                // convert the string to usable int
       break;
     case 'B':          // leading 'B' for BPM data
       inData = inData.substring(1);        // cut off the leading 'B'
       BPM = int(inData);                   // convert the string to usable int
       beat = true;                         // set beat flag to advance heart rate graph
       heart = 20;                          // begin heart image 'swell' timer
       break;
     case 'Q':            // leading 'Q' means IBI data 
       inData = inData.substring(1);        // cut off the leading 'Q'
       IBI = int(inData);     // convert the string to usable int
       break;
     default: 
       println("strange data:"+inData);
   }
  
  
}
