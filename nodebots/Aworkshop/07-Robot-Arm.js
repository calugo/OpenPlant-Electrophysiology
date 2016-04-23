var five = require("johnny-five"),
  board, potentiometer, servo;

board = new five.Board();

board.on("ready", function() {

  // Create a new `potentiometer` hardware instance.
  potentiometer = new five.Sensor({
    pin: "A2",
    //freq: 250
  });


  servo = new five.Servo({
      pin: 9,
    //center: true
   });

  // Inject the `sensor` hardware into
  // the Repl instance's context;
  // allows direct command line access
  board.repl.inject({
    pot: potentiometer
  });

  // "data" get the current reading from the potentiometer
  potentiometer.on("data", function() {
    console.log(this.value, this.raw);
   });

  //
   potentiometer.scale(0, 180).on("change", function() {
  	// this.value will reflect a scaling from 0-1023 to 0-180
     console.log( "HOLA" );
     console.log( this.value );
     servo.to( this.value );
   });




});

