var five = require("johnny-five"), board, motor;

board = new five.Board();

board.on("ready", function() {
  // Create a new `motor` hardware instance.
  motor = new five.Motor({
    pin: 9
  });


  // Motor Event API

  // "start" events fire when the motor is started.
  motor.on("start", function() {
    console.log("start", Date.now());

    // Demonstrate motor stop in 2 seconds
    board.wait(2000, function() {
      motor.stop();
    });
  });

  // "stop" events fire when the motor is stopped.
  motor.on("stop", function() {
    console.log("stop", Date.now());
     board.wait(1000,function(){
       motor.start(200);
     });
  });

  // Motor API

  // start([speed)
  // Start the motor. `isOn` property set to |true|
  // Takes an optional parameter `speed` [0-255]
  // to define the motor speed if a PWM Pin is
  // used to connect the motor.
  motor.start(200);

  //stop()
  // Stop the motor. `isOn` property set to |false|
});


