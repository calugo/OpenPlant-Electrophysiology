
void TemperatureLEDs() {
  uint16_t r, b;
    
    // Get the temperature
    //
    float t = EngduinoThermistor.temperature();
    
    if (t > lastTemp) {
      if (tempStage == 0 &&  t-lastTemp > 1) {
        tempStage ++;
        lastTemp = t;
      }
      else if(tempStage>0 &&  t-lastTemp > 0.2 && tempStage < 4) {
        tempStage ++;  
        lastTemp = t;
      }
    }
    
    else if (lastTemp - t > 0.2) {  
      lastTemp = t;
      if (tempStage > 0)
        tempStage --;
      if (tempStage == 0)
        lastTemp = TBLUE;
    }

    // Print the temperature
    Serial.print("Temp is: ");
    Serial.println(t);
    
    // Choose the next point in RGB space by temperature - this is a 
    // linear scale that maps from pure blue for the coldest to pure
    // red for the hottest, with intermediate temperatures
    // represented by the appropriate mixture of blue and red.
    //
    // We constrain the temperature so that the lowest temperature we
    // represent is TBLUE and the highest is TRED. MAX_BRIGHTNESS is
    // the maximum we can set an LED channel to.
    //
    r = (constrain(t - TBLUE, 0, TRANGE) * MAX_BRIGHTNESS) / TRANGE;
    b = (constrain(TRED - t,  0, TRANGE) * MAX_BRIGHTNESS) / TRANGE;
    
    // Now set the colour of the all the LEDs appropriately.
    EngduinoLEDs.setAll(OFF);
    switch (tempStage) {
      case 0:
        EngduinoLEDs.setLED(6, r, 0, b);
        EngduinoLEDs.setLED(7, r, 0, b);
        EngduinoLEDs.setLED(8, r, 0, b);
        EngduinoLEDs.setLED(9, r, 0, b);
        break;
      case 1:
        EngduinoLEDs.setLED(4, r, 0, b);
        EngduinoLEDs.setLED(5, r, 0, b);
        EngduinoLEDs.setLED(10, r, 0, b);
        break;
      case 2:
        EngduinoLEDs.setLED(3, r, 0, b);
        EngduinoLEDs.setLED(11, r, 0, b);
        break;
       case 3:
        EngduinoLEDs.setLED(1, r, 0, b);
        EngduinoLEDs.setLED(2, r, 0, b);
        EngduinoLEDs.setLED(12, r, 0, b);
        break;
      case 4:
        EngduinoLEDs.setLED(0, r, 0, b);
        EngduinoLEDs.setLED(13, r, 0, b);
        EngduinoLEDs.setLED(14, r, 0, b);
        EngduinoLEDs.setLED(15, r, 0, b);
        break; 
    }  
    //EngduinoLEDs.setAll(r, 0, b);
}

void takeBaseTemp() {
    
  TBLUE = EngduinoThermistor.temperature()+2;
  TRED = TBLUE + 6;
  TRANGE= TRED - TBLUE;  
  tempStage = 0;
  lastTemp = TBLUE;
}
