var five = require("johnny-five"),
  board = new five.Board();


/////////////////////////////////////////
///////////////////////////////////////
board.on("ready", function() {
  // Creates a piezo object and defines the pin to be used for the signal
  var piezo = new five.Piezo(8);

  // Injects the piezo into the repl
  board.repl.inject({
    piezo: piezo
  });


  // Plays the same song with a string representation
  piezo.play({
    // song is composed by a string of notes
    // a default beat is set, and the default octave is used
    // any invalid note is read as "no note"
    song: "E E E - E E E - E G C D E - C - F F F F F E E F - C -",
    beats: 1 / 4,
    tempo: 75
  });

///////////////////////////////////////


});
