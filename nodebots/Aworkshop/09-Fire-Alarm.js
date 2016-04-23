var five = require("johnny-five");
var board = new five.Board();
var cond=0;

board.on("ready", function() {
  var led = new five.Led(13);
  var button = new five.Button(5); 
  var piezo = new five.Piezo(9);
  var temperature = new five.Thermometer({
    controller: "TMP36",
    pin: "A0"
  });

  //led.off();
  temperature.on("data", function() {
    var tc=this.celsius;

    console.log(tc);
    if((tc>=50)&&(cond==0)){
     led.toggle();
     piezo.frequency(587,1);
     //cond=0;
     console.log("A")     
    }  
    if((tc<50) && (cond==0)){
	piezo.noTone();
	led.stop().off();
     }

    if((tc<50)&&(cond==1)){
       led.toggle();
       piezo.frequency(587,1);     
    }

     if((tc>=50)&&(cond==1)){
       led.stop().off();
       piezo.noTone();  
      // cond=0   
    } 
 
  });

  button.on("press", function() {
    console.log( "Button pressed" );
    cond=1;
  });
  
//led.off(); 

//Attach a temperature sensor TMP36 to **A0**
//  * Attach a piezo to pin **9**
//  * Attach an LED to pin **13**
//  * Attach a button to pin **5**


  // This will grant access to the led instance
  // from within the REPL that's created when
  // running this program.
// this.repl.inject({
//    led: led
//  });

//  led.blink();

});


