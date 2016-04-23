/**
* \defgroup EngduinoIR Driver for Engduino IR
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
#ifndef EngduinoIR_h
#define EngduinoIR_h

#include <Arduino.h>
#include <Engduino.h>

// Size of the send/receive buffers.
// IRBUFSZ is the max size that a caller of send/recv should
// use for a send/receive buffer.
//
// Note that the raw buffer is used to contain timings of
// the data bits and the start/stop bits, so is in effect just
// short of the max number of bits. The size is therefore longer.
// 
//
#define IRBUFSZ		12		// Length of the max packet size in bytes
#define RAWBUFSZ	100 	// Length of raw duration buffer

// Needed to ensure correct linkage between C++ and C linkage of ISRs
extern "C" void TIMER3_COMPB_vect(void) __attribute__ ((signal));
extern "C" void INT2_vect(void) __attribute__ ((signal));

#if defined(__BOARD_ENGDUINOV2)
	extern "C" void TIMER1_COMPB_vect(void) __attribute__ ((signal));
#elif defined(__BOARD_ENGDUINOV3)
	extern "C" void PCINT0_vect(void) __attribute__ ((signal));
#endif

class EngduinoIRClass
{
	private:
		volatile uint8_t  rcvstate;
		volatile uint8_t  rawlen;
		volatile uint16_t rawbuf[RAWBUFSZ];
		volatile bool     sending;

		void mark(uint16_t time);
		void space(uint16_t time);
				
	public:
		EngduinoIRClass();
		void begin();
		void end();
		
		void sendBit(bool b);
		void send(uint8_t b, bool startstop=true);
		void send(uint8_t *buf, unsigned int len, bool startstop=true);
		void send(char    *buf, unsigned int len, bool startstop=true);
		void sendRaw(unsigned int *buf, int len);
		
		int  recv(uint8_t *buf, uint16_t timeout=0, bool startstop=true);
		int  recvRaw(uint16_t *buf, uint16_t timeout=0);

		/*
		 * The ISR needs access to the private variables, so we declare it
		 * a friend of the class
		 */
		friend void TIMER3_COMPB_vect();
#if defined(__BOARD_ENGDUINOV1)
		friend void INT2_vect();
#elif defined(__BOARD_ENGDUINOV2)		
		friend void TIMER1_COMPB_vect();
		friend void INT2_vect();
#elif defined(__BOARD_ENGDUINOV3)		
		friend void PCINT0_vect();
#endif
};



// Receiver states
//
#define STATE_BLOCKED 1
#define STATE_IDLE    2
#define STATE_READING 3
#define STATE_STOP    4
#define STATE_TIMEOUT 5


// Between code gap
//
#define GAP 		  5000				// 5ms without a mark = counts between code gap

// Timer munging - delayMicroseconds waits for longer than it should by about
// the amount given in DELAYOFFSET.
//

#define DELAYOFFSET	   9					// Time that delayMicroseconds is actually late by, in us.
#if defined(__BOARD_ENGDUINOV1) || defined(__BOARD_ENGDUINOV3)
	#define BITTIME 	   (600-DELAYOFFSET)	// This should give us 500 us per bit
	#define MARKSPACESPLIT 1500
#elif defined(__BOARD_ENGDUINOV2)
	#define SPACETIME 	   (500-DELAYOFFSET)			// This should give us 500 us per bit
	#define MARKTIME 	   (50-DELAYOFFSET)				// This should give us 50 us per bit if it's a mark - but we send it twice
	#define MARKSPACESPLIT 0.9*(2*MARKTIME + SPACETIME)	// This is 90% of the least amount of time a space can occupy. Marks should be way below this.
#endif 
// Error codes
//
#define E_TIMEOUT	  -1
#define E_SYSERR	  -2

#define MARK  true
#define SPACE false

extern EngduinoIRClass EngduinoIR;

#endif

/** @} */
