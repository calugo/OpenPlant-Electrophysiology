var five = require("johnny-five"),
  board, photoresistor,led;

board = new five.Board();

board.on("ready", function() {

  // Create a new `photoresistor` hardware instance.
  photoresistor = new five.Sensor({
    pin: "A0",
   //freq: 250
  });

  led = new five.Led(9);

  // Inject the `sensor` hardware into
  // the Repl instance's context;
  // allows direct command line access
  board.repl.inject({
    pot: photoresistor
  });

  // "data" get the current reading from the photoresistor
  photoresistor.on("data", function() {
    console.log(this.value);
    if(this.value>600){
     led.on();
     }
    else{led.off();} 
    
  });
  led.off();
});
