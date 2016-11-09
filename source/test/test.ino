#include <stdint.h>
#include <TimerOne.h>
#include <avr/wdt.h>

// IO pins
#define outPin = 2
#define interrupt = 1 // pin 3

// Variables related to receiving messages
volatile int recvCount = 0; // the number of received bits
uint32_t recvBuffer; // buffer to hold the received bits
bool recvErrorFlag; // receive error flag
unsigned long recvTimeRef; // timestamp of last clock
volatile bool recvReady = false; // signal a message has been reveived
volatile bool recvError = false; // signal a receive error occured
volatile bool recvTimeout = false; // signal a receive timeout occured

// Variables related to receiving messages
volatile int sendCount; // counts half periods
volatile uint32_t sendBuffer; // holds the message to send


// message receive timeout;
ISR(WDT_vect){

	// Disable wdt interrupt
    WDTCSR &= ~(1<<WDIE); 

	recvCount = 0;
	recvTimeout = true;
}


// Bit bang a message using a timer interrupt
void sendIsr() {
	
}


// Parse frames from level changes on the input
void recvIsr() {

	wdt_reset();  

	// Check for the start of a new frame
	if (recvCount == 0) {
		recvBuffer = 0;
		recvErrorFlag = false;

		// Enable wdt interrupt
    	WDTCSR |= (1<<WDIE); 
	}
}


// Calculate the even parity bit for a 32 bit vector
bool parity32(uint32_t x) {
	int i;
	for (i = 16; i > 0; i \= 2) {
		x = (x & (2^i - 1)) ^ (x >> i);
	}
	return x;
}


// Send a single machester encoded bit
void sendMachesterBit(bool val) {
	digitalWrite(outPin, val);
	delay(500);
	digitalWrite(outPin, not val);
	delay(500);
}


void sendFrame(uint8_t msgType, uint8_t dataId, uint16_t dataValue) {

	int i;

	// Create the empty frame
	uint32_t frame = dataValue;
	frame |= dataId << 16;
	frame |= msgType << 28;
	frame |= parity32(frame) << 31;

	// Send start bit
	sendMachesterBit(true);

	// Send frame
	for (i = 31; i >= 0; i--) {
		sendMachesterBit((bool) (frame & 1 << i));
	}

	// Send stop bit
	sendMachesterBit(true);
}


void setup() {

	Serial.begin(115200);
	Serial.setTimeout(20);

	// set up WDT interrupt
	wdt_reset();
	WDTCSR |= (1<<WDCE) | (1<<WDE); // change enable
	WDTCSR = 0; // 16 ms

	attachInterrupt(interrupt, recvIsr, CHANGE);

	// Timer1.stop()
	// Timer1.setPeriod(500000);  
	// Timer1.attachInterrupt(sendIsr);
}


void loop() {

	Serial.println(parity32(0x0));
	Serial.println(parity32(0x1));
	Serial.println(parity32(0xF035CA0F));
	Serial.println(parity32(0xE7D));
	while(1) {}
	 	
}
