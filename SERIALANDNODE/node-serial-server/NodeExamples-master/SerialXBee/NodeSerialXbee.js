/*
	nodeSerialInXBee.js

	Takes in a serial stream from a Digi XBee 802.15.4 radio (XB24 firmware)

	This script expects a steady stream of input from the serial port
	separated by a byte with the value 0x7E.

	This is written as an exercise to see if I could interpret XBee API packets
	in node.js. This is not production-ready code.

	To call this from the command line:

	node serialTest.js portname

	where portname is the path to the serial port.

	created 22 Dec 2014
	modified 10 Jun 2015
	by Tom Igoe
*/

// serial port initialization:
var serialport = require('serialport'),		// include the serialport library
SerialPort  = serialport.SerialPort,			// make a local instance of serial
portName = process.argv[2],								// get the port name from the command line
portConfig = {
	baudRate: 9600
};

// open the serial port:
var myPort = new SerialPort(portName, portConfig);

// called when the serial port opens:
myPort.on('open', function() {
	console.log('port open');
	console.log('baud rate: ' + myPort.options.baudRate);

	// called when there's new incoming serial data:
	myPort.on('data', function (data) {
		var output = [];		// an array to hold the output

		for (c in data) {
			var value = data.charCodeAt(c);			// get the byte value
			if (value > 255) value = value >>8;	// if it's > 255, get the last 8 bits
			output.push(value);									// add it to the output array
		}

		parseData(output);						// run this through the parser
	});
});

// called when the serial port closes:
myPort.on('close', function() {
	console.log('port closed');
});

// called when there's an error with the serial port:
myPort.on('error', function(error) {
	console.log('there was an error with the serial port: ' + error);
	myPort.close();
});


/*
	This parser is adapted from one I wrote for an example in "Making Things Talk"
	(specifically https://github.com/tigoe/MakingThingsTalk2/blob/master/chapter7/project14/XbeePacketGrapher/XbeePacketGrapher.pde)

	The packet should be 22 bytes long,
 	made up of the following:
 	byte 1:     0x7E, the start byte value
 	byte 2-3:   packet size, a 2-byte value  (not used here)
 	byte 4:     API identifier value, a code that says what this response is (not used here)
 	byte 5-6:   Sender's address
 	byte 7:     signalStrength, Received Signal Strength Indicator (not used here)
 	byte 8:     Broadcast options (not used here)
 	byte 9:     Number of samples to follow
 	byte 10-11: Active channels indicator (not used here)
 	byte 12-21: 5 10-bit values, each ADC samples from the sender

	The Xbee radio sending this packet should have the following settings:
	ATMY (whatver address you want)
	ATDL (the address of the receiving radio)
	ATID (same PAN ID as the receiving radio)
	ATD0 2  (enable analog in on pin D0)
	ATIT 5 	(5 samples per packet)
	ATIR FF	(I used FF for the longest possible time between transmissions)
*/
function parseData(thisPacket) {
  // make sure the packet is 22 bytes long first:
  if (thisPacket.length >= 21) {
    var adcStart = 10;                     // ADC reading starts at byte 11
    var numSamples = thisPacket[7];        // number of samples in packet
    var adcValues = []; 				   // array to hold the 5 readings
    var total = 0;                         // sum of all the ADC readings

    // read the address. It's a two-byte value, so you
    // add the two bytes as follows:
    var address = thisPacket[4] + thisPacket[3] * 256;

    // read the received signal strength:
    var signalStrength = thisPacket[5];

    // read <numSamples> 10-bit analog values, two at a time
    // because each reading is two bytes long:
    for (var i = 0; i < numSamples * 2;  i=i+2) {
      // 10-bit value = high byte * 256 + low byte:
      var thisSample = (thisPacket[i + adcStart] * 256) +
        thisPacket[(i + 1) + adcStart];
      // put the result in one of 5 bytes:
      adcValues[i/2] = thisSample;
      // add the result to the total for averaging later:
      total = total + thisSample;
    }
    // average the result:
    var average = total / numSamples;
  }
  console.log("address: " + address
  	+ " RSSI: " + signalStrength
  	+ " samples: " + numSamples
  	+ " average: " + average);
}
