var five = require("johnny-five"),
    onButton, led;

five.Board().on("ready", function() {

  onbutton = new five.Button(5);
  led = new five.Led(9);
  
  // Button Event API
  onbutton.on("press", function(value) {
   console.log("down-on");
   led.toggle();
  });

  led.off(); 
});

