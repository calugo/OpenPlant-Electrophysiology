var dnode = require('dnode');
var five = require("johnny-five");

/////////
five.Board().on("ready", function() {
  var temperature = new five.Thermometer({
    controller: "TMP36",
    pin: "A0"
  });

  temperature.on("data", function() {
    //console.log(this.celsius + "°C";
  });

/////////
var server = dnode({
            getTemperature : function(cb) {
			 	cb(temperature.celsius)}
             });

server.listen(1337);

});
