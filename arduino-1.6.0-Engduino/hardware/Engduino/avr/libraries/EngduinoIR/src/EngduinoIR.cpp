/**
* \addtogroup EngduinoIR
*
* This is the driver code for IR on the Engduino
* On v1.0 this is the Vishay TFBS4711 chip. This driver
* makes use of a timer to measure the length of inter-mark
* gaps, and an edge-triggered ISR to know where the marks
* are.
* 
* Because the ISR for the LEDs occupies considerable time,
* this can run slowly and somewhat erratically, meaning that
* only a rather low data rate is possible. The code
* distinguishes between marks and spaces based on the
* relative timings of them, and the threshold is hardwired
* in the header file.
*
* This code makes use of timer 3, which cannot then be used
* for other purposes.
*
* @{
*/

/**
* \file 
* 		Engduino IR driver
* \author
* 		Engduino team: support@engduino.org
*/

//v3
#include "EngduinoIR.h"
#include <avr/interrupt.h>	// ISR
#include "pins_arduino.h"

#define DEBUGG 0
#if DEBUGG
	#define PRINTLN(...) Serial.println(__VA_ARGS__)
	#define PRINT(...) Serial.print(__VA_ARGS__)
#else if 
	#define PRINTLN(...)
	#define PRINT(...)
#endif
  
  
/*---------------------------------------------------------------------------*/
/**
* \brief Constructor
* 
* C++ constructor for this class. Empty.
*/
EngduinoIRClass::EngduinoIRClass()
{
}

/*---------------------------------------------------------------------------*/
/**
* \brief begin function - must be called before using other functions
*
* This function enables the IR, sets up the receive state machine,
* initialises timer 3 to run at 1MHz, to give high resolution timings, and
* sets the interrupts for the TFBS4711 IR device. The TFBS4711 chip will drive
* the IR_RX line low briefly when receiving a mark, and will not generate any
* event whilst receiving a space. Consequently, we can only tell the difference
* between them by looking at the gaps between two marks; longer gaps mean more
* spaces.
*
*/
void EngduinoIRClass::begin()
{
	pinMode(IR_SD, OUTPUT);    // Shutdown pin - active high
	pinMode(IR_TX, OUTPUT);    // Output to IR
	pinMode(IR_RX, INPUT);     // Input from IR

	digitalWrite(IR_TX, LOW);  // Set output to be space initially
	digitalWrite(IR_SD, HIGH); // We want it to be shutdown initially
    
	// Initialise state machine variables
	//
	rcvstate = STATE_IDLE;
	rawlen   = 0;
	sending  = false;
	
	// Set up receive timer
	//
	cli();					// Disable interrupts
	
	// Use timer 3 to figure identify marks and spaces in the received signal - by counting
	// periods between those in which there was a mark. If we know the expected baud rate, we 
	// can assess whether a mark or a space was transmitted. We set the clock to sample ten 
	// times as fast as the expected baud rate.
	//
	// We use the timer in CTC (Clear Timer on Compare match) mode in which the timer counts
	// up at a given frequency, stopping and being cleared when it reaches a predetermined count
    //
	// At 8MHz with a /8 prescaler, one count is equal to 1 us

	// Timer setup
	// The following are true for the Engduino:
	// * The clock runs at 8MHz
	// * There is a fixed minimum time to service an ISR of somewhere in the range 10-22us
	//   and taking an interrupt more frequently than this is pointless - even with a short ISR.
	// * Take care on minimum times with a longer ISR
	// * If we set the count to, n, then we will interrupt after n+1 clock cycles (@ 8MHz)
	//
	// For a 16 bit write we must write the high byte before the low byte.
	//
    TIMSK3 &= 0xF0;			  	// Disable interrupts for all timer 3 comparisons, plus overflow
    TCCR3A  = 0x00;			  	// Turn off OCnA connections, set up for CTC mode
	TCCR3B  = 0x08;			  	// CTC mode, clock off, no input capture
	OCR3AH  = 0xFF;  			// Set the counter to MAX for now
	OCR3AL  = 0xFF; 			// Set the counter to MAX for now
	TCNT3H  = 0x00;			  	// Reset the timer to zero
	TCNT3L  = 0x00;			  	// Reset the timer to zero
	TIMSK3 |= 0x04;      	  	// And enable interrupts for a compare B match only
#ifdef __BOARD_ENGDUINOV1
	// We use timer 1 with external pin T1 (connected to the IR RX line) to drive the counter
	// and time out when we get to a single count.
	// We get short (2.2.us) zero-going pulse (i.e. a falling edge) whenever there is a mark
	// NOTE: By default, we will get an interrupt when we are transmitting since the
	// data is looped back.
	//
    TIMSK1 &= 0xF0;			  	// Disable interrupts for all timer 1 comparisons, plus overflow
    TCCR1A  = 0x00;			  	// Turn off OCnA connections, set up for CTC mode
	TCCR1B  = 0x0E;			  	// CTC mode, input capture on T1 pin on falling edge
	OCR1AH  = 0x00;  			// Set the counter to 1
	OCR1AL  = 0x01; 			// Set the counter to 1
	TCNT1H  = 0x00;			  	// Reset the timer to zero
	TCNT1L  = 0x00;			  	// Reset the timer to zero
	TIMSK1 |= 0x04;      	  	// And enable interrupts for a compare B match only
#elif defined(__BOARD_ENGDUINOV2) 	
	// Switch on the interrupts on the IR_RX line and set them to function on a falling edge
	// We get short (2.2.us) zero-going pulse (i.e. a falling edge) whenever there is a mark
	// NOTE: By default, we will get an interrupt when we are transmitting since the
	// data is looped back.
	//
	TIMSK1 &= 0xF0;			  	// Disable interrupts for all timer 1 comparisons, plus overflow
    TCCR1A  = 0x00;			  	// Turn off OCnA connections, set up for CTC mode
	TCCR1B  = 0x0E;			  	// CTC mode, input capture on T1 pin on falling edge
	OCR1AH  = 0x00;  			// Set the counter to 1
	OCR1AL  = 0x01; 			// Set the counter to 1
	TCNT1H  = 0x00;			  	// Reset the timer to zero
	TCNT1L  = 0x00;			  	// Reset the timer to zero
	TIMSK1 |= 0x04;      	  	// And enable interrupts for a compare B match only
#elif defined(__BOARD_ENGDUINOV3) 
	// Switch on the interrupts on the IR_RX line and set them to function on a falling edge
	// We get short (2.2.us) zero-going pulse (i.e. a falling edge) whenever there is a mark
	// NOTE: By default, we will get an interrupt when we are transmitting since the
	// data is looped back.
	//
	PCICR  |= 0x01;				// Pin change interrupt 0 is enabled.
	PCMSK0	= 0x10;	// pin change interrupt is enabled on the corresponding PCINT4
#endif
	
	sei();  // Enable interrupts

	// And, finally, wake up the IR
	//
	digitalWrite(IR_SD, LOW);
}

/*---------------------------------------------------------------------------*/
/**
* \brief end function - switch off the IR
*
* Switch off both the IR chip and the ISRs.
*
*/
void EngduinoIRClass::end()
{
	digitalWrite(IR_SD, HIGH);	// Put the IR back to sleep
	TIMSK3 &= 0xFB;				// Disable the timer interrupt
#if defined(__BOARD_ENGDUINOV1)||defined(__BOARD_ENGDUINOV2)
	EIMSK  &= 0xFB;				// Disable the INT2 interrupt
#elif defined (__BOARD_ENGDUINOV3)
	PCICR  &= 0xFE;				// Disable the PCINT interrupt
#endif
}


/*---------------------------------------------------------------------------*/
/**
* \brief Send a single bit.
* \param b The boolean representing the bit to be sent.
*
* This sends a single bit. A mark is sent as a mark signal, and a space is
* sent as a space followed by a mark. This encoding works well because we do
* not have to concern ourselves about the relative timings of runs of spaces.
* Instead a mark - space transition should always be a constant length, and
* this should be considerably longer than a mark - mark transition.
*
*/
void EngduinoIRClass::sendBit(bool b)
{   
	bool sending_temp = sending;
	
	while (rcvstate == STATE_READING)		// Wait until we're not receiving
		;
	sending = true;
	
	if (b & 1) {
#if defined(__BOARD_ENGDUINOV1)|| defined(__BOARD_ENGDUINOV3)
		mark(BITTIME);
#elif defined(__BOARD_ENGDUINOV2)
		mark(MARKTIME);
		mark(MARKTIME);
#endif 
	}	
	else {
#if defined(__BOARD_ENGDUINOV1)|| defined(__BOARD_ENGDUINOV3)
		space(BITTIME);
		mark(BITTIME);
#elif defined(__BOARD_ENGDUINOV2)
		space(SPACETIME);
		mark(MARKTIME);
		mark(MARKTIME);
#endif
	}
	
	sending = sending_temp;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Send a byte
* \param b The byte to send
* \param startstop Whether to send mark bits as start/stop markers
*
* If startstop is true, send a mark at the beginning and end of data, but
* between the two, send the byte, LSB first.
*
*/
void EngduinoIRClass::send(uint8_t b, bool startstop)
{   
	bool sending_temp = sending;
	uint8_t mask = 0x01;

	while (rcvstate == STATE_READING)		// Wait until we're not receiving
		;
	sending = true;

	if (startstop)
		sendBit(MARK);
	
	for (int i = 0; i < 8; i++) {
		PRINT((b & mask) != 0);
		sendBit((b & mask) != 0);
		mask = mask << 1;
	}
	PRINTLN("");

	if (startstop) {
		sendBit(MARK);
		delayMicroseconds(2*GAP);
	}
	
	sending = sending_temp;	
}


/*---------------------------------------------------------------------------*/
/**
* \brief Send a buffer of bytes
* \param buf The buffer of bytes to send as a (uint8_t *)
* \param len Length of the buffer
* \param startstop Whether to send mark bits as start/stop markers
*
* If startstop is true, send a mark at the beginning and end of data, but
* between the two, send the individual bytes, LSB first.
*
*/
void EngduinoIRClass::send(uint8_t *buf, unsigned int len, bool startstop)
{   
	bool sending_temp = sending;
	
	while (rcvstate == STATE_READING)		// Wait until we're not receiving
		;
	sending = true;

	if (startstop)
		sendBit(MARK);
		
	for (int i = 0; i < len; i++)
		send(buf[i], false);

	if (startstop) {
		sendBit(MARK);
		delayMicroseconds(2*GAP);
	}

	sending = sending_temp;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Send a buffer of bytes
* \param buf The buffer of bytes to send as a (char *) 
* \param len Length of the buffer
* \param startstop Whether to send mark bits as start/stop markers
*
* If startstop is true, send a mark at the beginning and end of data, but
* between the two, send the individual bytes, LSB first.
*
*/
void EngduinoIRClass::send(char *buf, unsigned int len, bool startstop)
{	
	send((uint8_t *)buf, len, startstop);
}

/*---------------------------------------------------------------------------*/
/**
* \brief Raw send function. Provide timings for mark and space pairs
* \param buf Buffer containing alternate mark/space timings in microseconds 
* \param len Length of the buffer
*
* The argument to this function is a buffer containing alternate mark/space
* timings, given in milliseconds, allowing a higher-level protocol to be
* written in a sketch. 
*
*/
// Buffer contains times for mark/space pairs
//
void EngduinoIRClass::sendRaw(unsigned int *buf, int len)
{
	bool sending_temp = sending;
	
	while (rcvstate == STATE_READING)		// Wait until we're not receiving
		;
	sending = true;
	
	for (int i = 0; i < len; i++) {
		if ((i & 1) == 0)
			mark(buf[i]);
		else
			space(buf[i]);
	}
	
	space(0); // Just in case the list isn't an even length

	sending = sending_temp;	
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function to send a mark of a given length
* \param time The length of the mark in microseconds
*
* Send a mark, and send it for the given number of microseconds. Note that
* for this chip, a mark is sent on a rising edge of the IR_TX line. It
* persists for the time that the line is high - except that if this is
* longer than 100us, the pulse is trimmed to 100us.
*
*/
void EngduinoIRClass::mark(uint16_t time)
{
	// We always pull IR_TX low for a short period before the end of the
	// bit (note: there's a fixed overhead in the delayMicroseconds call)
	// Anyway, if we do this, we have a clean rising edge if there's a
	// mark immediately following. It's down to us to ensure that 
	//
	digitalWrite(IR_TX, HIGH);						// Transmit mark
	delayMicroseconds(time - DELAYOFFSET - 1);		// Leave high for the given time minus a period of (DELAYOFFSET+1)
	digitalWrite(IR_TX, LOW);						// Then pull low 
	delayMicroseconds(1);							// And delay for around (DELAYOFFSET+1)
}

/*---------------------------------------------------------------------------*/
/**
* \brief Internal function to send a space of a given length
* \param time The length of the space in microseconds
*
* A space is the absence of a mark, so just delay and send nothing for
* the given time.
*
*/
void EngduinoIRClass::space(uint16_t time)
{
	// Sends an IR space for the specified number of microseconds.
	// We have a space when IR_TX is low, which it is by default, so
	// we just delay.
	//
	delayMicroseconds(time);
}
#ifdef __BOARD_ENGDUINOV1
/*---------------------------------------------------------------------------*/
/**
* \brief ISR2 interrupt service routine, called when a falling edge is seen on
*        the RX line
*
* Set for a falling edge on the INT2 pin - i.e. on IR_RX. Such events occur
* in 2.2us periods when there is a mark on the IR. The width of the pulse is
* unrelated to the baud rate of transmission, so we simply time the space
* between marks using timer 3, setting (and resetting) a timeout which, when
* it expires, indicates that the code has ended.
*
*/
ISR(INT2_vect)
{	if (EngduinoIR.sending)				// The IR we send is reflected in the receive.
		return;							// Ignore it

	if (EngduinoIR.rawlen >= RAWBUFSZ) {
		EngduinoIR.rcvstate = STATE_STOP;
	}

	switch (EngduinoIR.rcvstate) {
		case STATE_BLOCKED:
 		case STATE_IDLE:
 			TCCR3B &= 0xF8;				// Switch off clock.

 			EngduinoIR.rawlen   = 0;
 			EngduinoIR.rcvstate = STATE_READING;

			// Now set a timeout to detect an inter-code space.
			// For a 16 bit write we must write the high byte before the low byte.
			//
			OCR3AH  = (GAP-1) >> 8;  	// Set the counter to be the given number of counts
			OCR3AL  = (GAP-1)  % 256; 	// Set the counter to be the given number of counts
			TCNT3H  = 0x00;			  	// Reset the timer to zero
			TCNT3L  = 0x00;			  	// Reset the timer to zero
			TCCR3B |= 0x02;				// And turn clock back on
			break;
			
		case STATE_READING:
 			TCCR3B &= 0xF8;				// Switch off clock.
 			
 			{ uint16_t l = TCNT3L;		// For a 16 bit read, we must read the low byte before the high byte
 			  uint16_t h = (TCNT3H << 8);
 			 EngduinoIR.rawbuf[EngduinoIR.rawlen++] = h + l;
 			}
										// For a 16 bit write we must write the high byte before the low byte.
			TCNT3H  = 0x00;			  	// Reset the timer to zero
			TCNT3L  = 0x00;			  	// Reset the timer to zero
			TCCR3B |= 0x02;				// Turn clock back on
			break;
			
		case STATE_STOP:
			break;
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief ISR3 interrupt service routine, called when the timer has expired.
*
* TIMER3 interrupt code - timer 3 COMPB is used as a timeout to determine
* when a code we are receiving has ended, or when a call to the recv function
* has timed out. 
*
*/
ISR(TIMER3_COMPB_vect)
{   
	TCCR3B  &= 0xF8;					// Switch off clock.

	switch (EngduinoIR.rcvstate) {
		case STATE_READING: 
			EngduinoIR.rcvstate = STATE_STOP;		// We timed out, so the code must have ended
			break;
		case STATE_BLOCKED:
			EngduinoIR.rcvstate = STATE_TIMEOUT;	// We just timed out waiting for an input
			break;
	}
}
#elif defined(__BOARD_ENGDUINOV2)
/*---------------------------------------------------------------------------*/
/**
* \brief TIMER3 interrupt service routine, called when the timer has expired.
*
* TIMER3 interrupt code - timer 3 COMPB is used as a timeout to determine
* when j code we are receiving has ended, or when a call to the recv function
* has timed out. 
*
*/
ISR(TIMER3_COMPB_vect)
{   
	TCCR3B  &= 0xF8;					// Switch off clock.

	switch (EngduinoIR.rcvstate) {
		case STATE_READING: 
			EngduinoIR.rcvstate = STATE_STOP;		// We timed out, so the code must have ended
			break;
		case STATE_BLOCKED:
			EngduinoIR.rcvstate = STATE_TIMEOUT;	// We just timed out waiting for an input
			break;
	}
}

/*---------------------------------------------------------------------------*/
/**
* \brief TIMER1 interrupt service routine, called when the timer has expired.
*
* TIMER1 interrupt code - Set for an overflowed count driven by the T1 pin
* which is connected to IR_RX. Such events occur in 2.2us periods when there
* is a mark on the IR. The width of the pulse is unrelated to the baud rate
* of transmission, so we simply time the space between marks using timer 3,
* setting (and resetting) a timeout which, when it expires, indicates that
* the code has ended.
* 
* It is necessary for us to have marks come in pairs to drive this ISR
* correctly - the first mark counts from 0-1, generating an interrupt, the
* second resets the timer counter to 0.
* 
* Note: This approach is dictated by the connections on the board. It means
* that we cannot use a remote control with the Engduino v2 because that will
* not generate the marks in pairs as we have done above.
*
*/
ISR(TIMER1_COMPB_vect)
{	if (EngduinoIR.sending)				// The IR we send is reflected in the receive.
		return;							// Ignore it

	if (EngduinoIR.rawlen >= RAWBUFSZ) {
		EngduinoIR.rcvstate = STATE_STOP;
	}

	switch (EngduinoIR.rcvstate) {
		case STATE_BLOCKED:
 		case STATE_IDLE:
 			TCCR3B &= 0xF8;				// Switch off clock.

 			EngduinoIR.rawlen   = 0;
 			EngduinoIR.rcvstate = STATE_READING;

			// Now set a timeout to detect an inter-code space.
			// For a 16 bit write we must write the high byte before the low byte.
			//
			OCR3AH  = (GAP-1) >> 8;  	// Set the counter to be the given number of counts
			OCR3AL  = (GAP-1)  % 256; 	// Set the counter to be the given number of counts
			TCNT3H  = 0x00;			  	// Reset the timer to zero
			TCNT3L  = 0x00;			  	// Reset the timer to zero
			TCCR3B |= 0x02;				// And turn clock back on
			break;
			
		case STATE_READING:
 			TCCR3B &= 0xF8;				// Switch off clock.
 			
 			{ uint16_t l = TCNT3L;		// For a 16 bit read, we must read the low byte before the high byte
 			  uint16_t h = (TCNT3H << 8);
 			  EngduinoIR.rawbuf[EngduinoIR.rawlen++] = h + l;
 			}
										// For a 16 bit write we must write the high byte before the low byte.
			TCNT3H  = 0x00;			  	// Reset the timer to zero
			TCNT3L  = 0x00;			  	// Reset the timer to zero
			TCCR3B |= 0x02;				// Turn clock back on
			break;
			
		case STATE_STOP:
			break;
	}
}

#elif defined(__BOARD_ENGDUINOV3)
/*---------------------------------------------------------------------------*/
/**
* \brief ISR2 interrupt service routine, called when a falling edge is seen on
*        the RX line
*
* Set for a falling edge on the INT2 pin - i.e. on IR_RX. Such events occur
* in 2.2us periods when there is a mark on the IR. The width of the pulse is
* unrelated to the baud rate of transmission, so we simply time the space
* between marks using timer 3, setting (and resetting) a timeout which, when
* it expires, indicates that the code has ended.
*
*/
ISR(PCINT0_vect)
{
	if (EngduinoIR.sending)				// The IR we send is reflected in the receive.
		return;							// Ignore it

	if (EngduinoIR.rawlen >= RAWBUFSZ) {
		EngduinoIR.rcvstate = STATE_STOP;
	}

	switch (EngduinoIR.rcvstate) {
		case STATE_BLOCKED:
 		case STATE_IDLE:
 			TCCR3B &= 0xF8;				// Switch off clock.

 			EngduinoIR.rawlen   = 0;
 			EngduinoIR.rcvstate = STATE_READING;

			// Now set a timeout to detect an inter-code space.
			// For a 16 bit write we must write the high byte before the low byte.
			//
			OCR3AH  = (GAP-1) >> 8;  	// Set the counter to be the given number of counts
			OCR3AL  = (GAP-1)  % 256; 	// Set the counter to be the given number of counts
			TCNT3H  = 0x00;			  	// Reset the timer to zero
			TCNT3L  = 0x00;			  	// Reset the timer to zero
			TCCR3B |= 0x02;				// And turn clock back on
			break;
			
		case STATE_READING:
 			TCCR3B &= 0xF8;				// Switch off clock.
 			
 			{ uint16_t l = TCNT3L;		// For a 16 bit read, we must read the low byte before the high byte
 			  uint16_t h = (TCNT3H << 8);
 			 EngduinoIR.rawbuf[EngduinoIR.rawlen++] = h + l;
 			}
										// For a 16 bit write we must write the high byte before the low byte.
			TCNT3H  = 0x00;			  	// Reset the timer to zero
			TCNT3L  = 0x00;			  	// Reset the timer to zero
			TCCR3B |= 0x02;				// Turn clock back on
			break;
			
		case STATE_STOP:
			break;
	}
	delayMicroseconds(2);
	PCIFR = 0xFF;
}

/*---------------------------------------------------------------------------*/
/**
* \brief ISR3 interrupt service routine, called when the timer has expired.
*
* TIMER3 interrupt code - timer 3 COMPB is used as a timeout to determine
* when a code we are receiving has ended, or when a call to the recv function
* has timed out. 
*
*/
ISR(TIMER3_COMPB_vect)
{   
	TCCR3B  &= 0xF8;					// Switch off clock.

	switch (EngduinoIR.rcvstate) {
		case STATE_READING: 
			EngduinoIR.rcvstate = STATE_STOP;		// We timed out, so the code must have ended
			break;
		case STATE_BLOCKED:
			EngduinoIR.rcvstate = STATE_TIMEOUT;	// We just timed out waiting for an input
			break;
	}
}
#endif
/*---------------------------------------------------------------------------*/
/**
* \brief Blocking receive of an IR transmission, with optional timeout
* \param buf The buffer in which to place the received data
* \param timeout The time to wait before returning
* \param startstop Whether to remove start/stop bits from transmission
* \return The length of the buffer in bytes.
*
* Receive a message if it's there to be received. Otherwise wait for a time
* that depends on the timeout value: if this is zero (as it is by default),
* then wait forever, else set a timer. If the timer expires return a negative
* number (-1), else return the length of the buffer received. The timeout is
* considered not to have expired if, by the time the period is passed, a
* message has *started* to arrive.
* 
* The first received bit is placed in the MSB, meaning that if there is not
* a complete byte's worth of data, the lowest bits will be zero.
* 
* Note: Differentiating between a mark and a space is based on a hardwired
* test of length. 
* 
*/
int EngduinoIRClass::recv(uint8_t *buf, uint16_t timeout, bool startstop)
{
	static uint16_t raw[RAWBUFSZ];
	uint8_t b = 0;		// The byte we're building
	uint8_t l = 0;		// length of the buffer we've built
	
	int len = recvRaw(raw, timeout);
	
	if (len < 0)
		return len;

	if (startstop)
		len = len - 1;	// Remove the stop bit.
						// The start bit isn't represented directly - it starts the timer.
  	
	for (int i = 0; i < len; i++) {
		if (raw[i] < MARKSPACESPLIT) {
			// It's a mark.
			// Shift in the appropriate one
			//
			b |= (1 << (i%8));		
		}
	
		if ((i+1)%8 == 0) {
			buf[l++] = b;
			b = 0;
		}
	}

	// If there's anything left, add it to the
	// list
	if ((len % 8) != 0) {
		buf[l++] = b;
	}

	return l;
}

/*---------------------------------------------------------------------------*/
/**
* \brief Raw receive function - returns timings for inter-mark gaps
* \param buf The buffer in which to place the inter-mark timings
* \param timeout The time to wait before returning
* \return The length of the buffer in bytes.
*
* Receive a message if it's there to be received. Otherwise wait for a time
* that depends on the timeout value: if this is zero (as it is by default),
* then wait forever, else set a timer. If the timer expires return a negative
* number (-1), else return the length of the buffer received. The timeout is
* considered not to have expired if, by the time the period is passed, a
* message has *started* to arrive.
* 
* On return, the buffer contains the timings, in microseconds, between pairs
* of marks. Again, this allows a higher-level protocol to be written in a
* sketch. 
*
*/
int EngduinoIRClass::recvRaw(uint16_t *buf, uint16_t timeout)
{
	int retval;
	
	switch (rcvstate) {
		case STATE_IDLE:
			// We're currently waiting for a code to arrive. Set a timeout
			// and wait either for this to expire or for a code to be received
			//
			rcvstate = STATE_BLOCKED;
	
			if (timeout > 0) {
				// Now set a timeout - the ISR can't be using it to look for an end-of-code gap
				// because the state is IDLE
				// For a 16 bit write we must write the high byte before the low byte.
				//
	 			TCCR3B &= 0xF8;					// Switch off clock.
				OCR3AH  = (timeout-1) >> 8;  	// Set the counter to be the given number of counts
				OCR3AL  = (timeout-1)  % 256; 	// Set the counter to be the given number of counts
				TCNT3H  = 0x00;			  		// Reset the timer to zero so we don't miss any events
				TCNT3L  = 0x00;			  		// Reset the timer to zero so we don't miss any events
				TCCR3B |= 0x02;					// Turn clock back on
			}
			
			// Block until either the timeout expires or there is something to receive
			//
			while (true) {
				switch (rcvstate) {
					case STATE_BLOCKED:
						continue;
						
					case STATE_TIMEOUT:
						rawlen   = 0;
						rcvstate = STATE_IDLE;
						return E_TIMEOUT;
						
					case STATE_STOP:
						// Copy the received code and set up for the next code
						//
						for (int i = 0; i < rawlen; i++)
							buf[i] = rawbuf[i];

						retval   = (int)rawlen;
						rawlen   = 0;
						rcvstate = STATE_IDLE;
						return retval;
						
					default:
						// We must have started to receive a code
						// The timeout will have been reset so
						// just wait until the code is received.
						//
						continue;
						
				}
			}
			

		default:
			// We're in the middle of receiving a message - just wait until it has arrived
			//
			while (rcvstate != STATE_STOP) {
				;
			}
			
			// Copy the received code and set up for the next code
			//
			for (int i = 0; i < rawlen; i++)
				buf[i] = rawbuf[i];			

			retval   = (int)rawlen;
			rawlen   = 0;
			rcvstate = STATE_IDLE;
			return retval;
	}
}



/*---------------------------------------------------------------------------*/
/*
 * Preinstantiate Objects
 */ 
EngduinoIRClass EngduinoIR = EngduinoIRClass();

/** @} */
