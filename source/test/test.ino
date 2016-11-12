#include <stdint.h>
#include <math.h>
#include <TimerOne.h>
#include <avr/wdt.h>

// IO pins
#define outPin 2
#define inPin 3
#define interruptNr 1 // interrupt number for pin 3

// Check if the time between X and Y in microseconds fall with bounds of T1 and T2
// Which are the half and whole period for the opentherm manchester encoding (0.5ms, 1ms)
#define CHECKT1(X, Y) ((Y-X) > 400 && (Y-X) < 650) 
#define CHECKT2(X, Y) ((Y-X) > 900 && (Y-X) < 1150)

// Variables related to receiving messages
volatile int recvCount = 0; // the number of received bits
volatile bool recvBusy = false; /// indicate a receive is in progress
uint32_t recvBuffer; // buffer to hold the received bits
volatile bool recvErrorFlag; // receive error flag
unsigned long recvTimeRef; // timestamp of last clock
bool recvHalf = false; // signal the last detected transition was a half period transition
volatile bool recvReady = false; // signal a message has been reveived
volatile bool recvError = false; // signal a receive error occured
volatile bool recvTimeout = false; // signal a receive timeout occured


// message receive timeout;
ISR(WDT_vect){
    
    // Disable wdt interrupt
  WDTCSR &= ~(1<<WDIE); 

  // reset receive state
    recvBusy = false;

    // signal a error or timeout to the event loop
    if (recvErrorFlag == true) {
        recvErrorFlag = false; // reset error flag
        recvError = true;
    } else {
        recvTimeout = true;
    }
}


// Parse manchester encoded frames from the opentherm interface.
// Each frame consists of a 32 bit message together with positive start and stop bits
void recvIsr() {

    bool checkT1;
    unsigned long t = micros();

    wdt_reset();  

    // If an error happened, discard all data and wait for a timeout
    if (recvErrorFlag == true)
        return;

    // Discard the first transition of the start bit and initialize variables
    if (recvBusy == false) {
        recvBusy = true;
        recvCount = 0;
        recvHalf = false;
        recvBuffer = 0x00000000;
        WDTCSR |= (1<<WDIE); // Enable wdt interrupt
        return;
    } 
    // First clock transition of the start bit
    else if (recvCount == 0) {
        if (!CHECKT1(recvTimeRef, t)) {
            recvErrorFlag = true;
        }
    } 
    // All other transitions
    else {
        checkT1 = CHECKT1(recvTimeRef, t);
        // First half period transitions
        if (recvHalf == false && checkT1) {
            recvHalf = true;
        } 
        // Second half period and full period transitions
        else if (recvHalf == true  && checkT1 || 
                 recvHalf == false && CHECKT2(recvTimeRef, t)) {
            // End of frame 
            if (recvCount++ == 32) {
                // Check stop bit 
                if (!digitalRead(inPin)) {
                    WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt
                    recvReady = true;
                } else {
                    recvErrorFlag = true;
                }
            } 
            // Store data bit
            else {
                recvBuffer = recvBuffer << 1;
                recvBuffer |= ~digitalRead(inPin);
                recvHalf = false;
            }
        } 
        // Timing error
        else {
            recvErrorFlag = true; 
        }
    }

    recvTimeRef = t;
}


// Calculate the even parity bit for a 32 bit vector
bool parity32(uint32_t x) {
    int i;
    for (i = 16; i > 0; i >> 1) {
        x = (x & (uint32_t) ((pow(2, i) - 1))) ^ (x >> i);
    }
    return x;
}


// Send a single machester encoded bit.
void sendMachesterBit(int val) {
    digitalWrite(outPin, val);
    delay(500);
    digitalWrite(outPin, not val);
    delay(500);
}


void sendFrame(uint8_t msgType, uint8_t dataId, uint16_t dataValue) {

    int mask = 0x80000000;

    // Create the message
    uint32_t msg = dataValue;
    msg |= dataId << 16;
    msg |= msgType << 28;
    msg |= parity32(msg) << 31;

    // Send start bit
    sendMachesterBit(true);

    // Send message
    while (mask != 0) {
        sendMachesterBit(msg & mask);
        mask = mask >> 1;
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

    attachInterrupt(interruptNr, recvIsr, CHANGE);
}


void loop() {

    uint32_t msgBuffer;
        char printBuffer[100];
    unsigned long tRef;

    Serial.println(parity32(0x0));
    Serial.println(parity32(0x1));
    Serial.println(parity32(0xF035CA0F));
    Serial.println(parity32(0xE7D));

    while(1) {
        
        // ask boiler water temperature
        sendFrame(0, 25, 0);

        // Listen for a reply for 1s
        tRef = millis();
        while(millis() - tRef < 1000) {

            if (recvError) {
                recvError = false;
                Serial.println('error');
            } 
            else if (recvTimeout) {
                recvTimeout = false;
                Serial.println('timeout');
            } 
            else if (recvReady) {
                recvReady = false;
                msgBuffer = recvBuffer;
                Serial.println('received message:'); 
                sprintf(printBuffer, "type = 0x, id = %x, data = %x", msgBuffer & 0x70000000, 
                                              msgBuffer & 0x00ff0000,
                                              msgBuffer & 0x0000ffff);
                                Serial.println(printBuffer); 
            }
        }
    }
}
