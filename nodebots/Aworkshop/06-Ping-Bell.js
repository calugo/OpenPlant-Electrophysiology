var five = require("johnny-five"),
  board = new five.Board();

var dgram = require("dgram");

/////////////////////////////////////////
///////////////////////////////////////
board.on("ready", function() {
  // Creates a piezo object and defines the pin to be used for the signal
  var piezo = new five.Piezo(8);

  // Injects the piezo into the repl
  board.repl.inject({
    piezo: piezo
  });


var server = dgram.createSocket("udp4");

server.on("error", function (err) {
  console.log("server error:\n" + err.stack);
  server.close();
});

server.on("message", function (msg, rinfo) {
  console.log("server got: " + msg + " from " +
    rinfo.address + ":" + rinfo.port);

///////////////////////////////////////
  // Plays the same song with a string representation
  piezo.play({
    // song is composed by a string of notes
    // a default beat is set, and the default octave is used
    // any invalid note is read as "no note"
    song: "C D F D A - A A A A G G G G - - C D F D G - G G G G F F F F - -",
    beats: 1 / 4,
    tempo: 100
  });

///////////////////////////////////////

});

server.on("listening", function () {
  var address = server.address();
  console.log("server listening " +
      address.address + ":" + address.port);
});

server.bind(1337);
// server listening 0.0.0.0:41234

});
