import processing.serial.*;

Serial myPort;  // Create object from Serial class
int Green = 0;
int val;
int Calibration = 0;



void setup() 
{
  size (400, 300); //canvas size

  PFont f;
  f = createFont("ArialItalics", 16, true);
  fill (0);
  {
    textFont(f, 16); // size
    text("X/Y", 50, 150+8);
    text("Z", 350, 150+8);

    textFont(f, 16); // size
    text ("Manipulator Control v1.0 by T Baden, CIN, Tübingen", 20, 30);

    textFont(f, 12); // size
    text ("(use keyboard: this window is for reference only!)", 110, 50);

    text("S", 150-45, 150+6);
    text("W", 150-45, 250+6);
    text("A", 100-45, 200+6);
    text("D", 200-45, 200+6);

    text("Q", 300-45, 220+6);
    text("E", 300-45, 150+6);

    text("1", 60-6, 70);
    text("2", 100-6, 70);
    text("Speed", 122, 96);


    text("press 'C' for calibration mode", 220, 290);
  }

  fill (0, Green, 255); 
  {
    ellipse (150, 150, 50, 50); // y up
    ellipse (150, 250, 50, 50); // y down
    ellipse (100, 200, 50, 50); // left
  

    ellipse (300, 220, 50, 50); // down
    ellipse (300, 150, 50, 50); // up
  }
  fill (125, Green, 0);
  ellipse (60, 90, 30, 30); // SPEED
  fill (255, Green, 0);
  ellipse (100, 90, 30, 30); // SPEED2

  String portName = Serial.list()[0]; 
  myPort = new Serial(this, portName, 9600);
}

void draw() {

  if (keyPressed == true)
  {
    if (key == 'a') {
      myPort.write('2');        // X left
      fill (255, Green, 0); 
      ellipse (100, 200, 50, 50);
    } 
    if (key == 'd') {
      myPort.write('1');         // X right
      fill (255, Green, 0); 
      ellipse (200, 200, 50, 50);
    } 

    if (key == 's') {
      myPort.write('4');         // Y Down
      fill (255, Green, 0); 
      ellipse (150, 250, 50, 50);
    } 
    if (key == 'w') {
      myPort.write('3');         // Y Up
      fill (255, Green, 0); 
      ellipse (150, 150, 50, 50);
    }
    if (key == 'e') {
      myPort.write('5');         // Z up
      fill (255, Green, 0); 
      ellipse (300, 150, 50, 50);
    } 
    if (key == 'q') {
      myPort.write('6');         // Z downad
      fill (255, Green, 0); 
      ellipse (300, 220, 50, 50);
    }
    /// SPEED
    if (key == '1') {
      myPort.write('7'); 
      Green = 0;     
      Calibration = 0;     
      println("Speed 1");
      fill (255, Green, 0);
      ellipse (60, 90, 30, 30); // SPEED
      fill (125, Green, 0);
      ellipse (100, 90, 30, 30); // SPEED2
    }
    if (key == '2') {
      myPort.write('8'); 

      Green = 0;    
      Calibration = 0;     
      println("Speed 2");
      fill (125, Green, 0);
      ellipse (60, 90, 30, 30); // SPEED
      fill (255, Green, 0);
      ellipse (100, 90, 30, 30); // SPEED2
    }
    /// CALIBRATION
    if (key == 'c') {
      myPort.write('9');  
      Green = 255;   
      Calibration = 1;   
      println("Calbration mode");
    }
  }
  else {
    myPort.write('0');
    //println("stop");
    fill (0, Green, 255); 
    {
      ellipse (150, 150, 50, 50); // y up
      ellipse (150, 250, 50, 50); // y down
      ellipse (100, 200, 50, 50); // left
      ellipse (200, 200, 50, 50); // right

      ellipse (300, 220, 50, 50); // down
      ellipse (300, 150, 50, 50); // up
    }
  }
  /// READ FROM ARDUINO


  if ( myPort.available() > 0)
  {
    val = myPort.read();
    println(val);
    Calibration = 0;
    Green = 0;
  }
}

