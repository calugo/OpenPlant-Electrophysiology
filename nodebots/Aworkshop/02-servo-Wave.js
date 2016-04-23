var five = require('johnny-five')
var board = new five.Board()      

board.on('ready', function (){
    
// Create a standard servo...
// 
//   - attached to pin 10
//   - centered
//
	var servo = new five.Servo({
  	pin: 9,
  	//center: true
	});

 	servo.sweep();

	board.wait(3000, function() {
    	    servo.stop();
	    servo.center()
  	});    
})


