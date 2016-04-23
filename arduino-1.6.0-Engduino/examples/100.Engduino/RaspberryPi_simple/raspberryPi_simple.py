import serial, time
import os

# This is a very simple Python program that demonstrates that it is possible
# to communicate with the Engduino over the serial link - both to send and
# receive values.
#
# The intent behind this example is to show that the Engduino can be used
# very simply in conjunction with the R-Pi as an output device through the LEDs,
# as an input device by having the Engduino return values from its sensors.
# Whilst not shown here, it also has the potential to be a communications device
# which could make the R-Pi a gateway between IR and, say, Ethernet. 
#


# Create and open an appropriate serial link
engduino = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
engduino.open()

# Now loop forever, waiting for a user command and sending that to the Engduino
# over the serial link. If the user types CTRL-C, then quit and close the
# connection cleanly.
#
try:
        while True:
                # Print the choices and wait for the input
                #
                print "Hello what would you like to do?"
                print "1. red"
                print "2. green"
                print "3. blue"
                print "4. off"
                print "5. temp"		
                choice = raw_input("> ")

                # Send the appropriate output to the Engduino given the choice
                #
                if choice == "1" or choice == "red":
                        engduino.write('r')
                elif choice == "2" or choice == "green":
                        engduino.write('g')
                elif choice == "3" or choice == "blue":
                        engduino.write('b')
                elif choice == "4" or choice == "off":
                        engduino.write('o')
                elif choice == "5" or choice == "temp":
                        engduino.write('t')
                        
                        # In this case there is also a return value from the
                        # Engduino, so wait for that.
                        temp = engduino.readline()
                        print "Temp is " + str(temp)

except KeyboardInterrupt:
        # Close the connection cleanly if CTRL-C is typed.
        engduino.close()
