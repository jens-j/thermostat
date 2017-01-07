#include <stdint.h>
#include <math.h>
#include <TimerOne.h>
#include <avr/wdt.h>

// IO pins
#define outPin 2
#define inPin 3
#define interruptNr 1 // interrupt number for pin 3
#define BUFFER_SIZE 40

// Check if the time between X and Y in microseconds falls with bounds of T1 and T2
// Which are the half and whole period for the opentherm manchester encoding (0.5ms, 1ms)
#define CHECKT1(X, Y) ((Y-X) > 400 && (Y-X) < 650) 
#define CHECKT2(X, Y) ((Y-X) > 900 && (Y-X) < 1150)

enum ErrorCode {NONE, WRONG_FIRST_EDGE, WRONG_SECOND_EDGE, EDGE_EARLY, EDGE_LATE, TIMEOUT};

// Variables related to receiving messages
volatile int recvCount = 0; // the number of received bits
volatile int recvCountBuffer;
volatile bool recvBusy = false; // indicate a receive is in progress
volatile uint32_t long recvData; // buffer to hold the received bits
volatile uint32_t long recvDataBuffer;
volatile bool recvErrorFlag; // receive error flag
volatile uint32_t recvTimeRef; // timestamp of last clock transition
volatile bool recvReady = false; // signal a message has been reveived
volatile bool recvError = false; // signal a receive error occured
volatile ErrorCode errorCode;
volatile ErrorCode errorCodeBuffer;
volatile bool midBitFlag; // flag mid cycle transition has occurred this cycle


// message receive timeout;
ISR(WDT_vect){
    
    // Disable wdt interrupt
    WDTCSR &= ~(1<<WDIE); 

    // Buffer data
    recvCountBuffer = recvCount;
    recvDataBuffer = recvData;

    errorCodeBuffer = (errorCode == NONE) ? TIMEOUT : errorCode;

    recvBusy = false; // reset receive state
    recvErrorFlag = false; // reset error flag
    recvError = true; // signal an error to the print loop
}


// Parse manchester encoded frames from the opentherm interface.
// Each frame consists of a 32 bit message together with positive start and stop bits
void recvIsr() {

    char printBuffer[80];
    sprintf(printBuffer, "inpin = %d", digitalRead(inPin));
    Serial.println(printBuffer);

    // uint32_t t = micros();
    // wdt_reset();

    // // If an error happened, discard all data and wait for a timeout
    // if (recvErrorFlag == true) {
    //     recvCount++;
    //     return;
    // }

    // // Discard the first transition of the start bit and initialize variables
    // if (recvBusy == false) {
    //     recvBusy = true;
    //     recvCount = 0;
    //     recvData = 0x00000001;
    //     midBitFlag = false;
    //     errorCode = NONE;
    //     WDTCSR |= (1<<WDIE); // Enable wdt interrupt

    //     // the first edge should always be positive
    //     if (digitalRead(inPin) == LOW) {
    //         recvErrorFlag = true;
    //         errorCode = WRONG_FIRST_EDGE;
    //     }
    // } 
    // // First clock transition of the start bit
    // else if (recvCount == 0) {

    //     // this is the messages first bit transition
    //     recvTimeRef = t;
        
    //     // // the second edge should always be negative
    //     // if (digitalRead(inPin) == HIGH) {
    //     //     recvErrorFlag = true;
    //     //     errorCode = WRONG_SECOND_EDGE;
    //     // }

    //     recvCount++;
    //     recvData = (recvData << 1) | ~digitalRead(inPin);

    // } 
    // // All other transitions
    // else {
    //     // check for half cycle transitions 
    //     if (t - recvTimeRef < 900) {
    //         if (midBitFlag == false) {
    //             midBitFlag = true;
    //             return;
    //         } else {
    //             recvErrorFlag = false;
    //             errorCode = EDGE_EARLY;
    //         }
    //     } 
    //     // valid bit transition
    //     else if (t - recvTimeRef < 1150) {
    //         recvData = (recvData << 1) | ~digitalRead(inPin);
    //         if (++recvCount == 34) {
    //             WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt
    //             recvReady = true;
    //         }
    //         recvTimeRef = t;
    //         midBitFlag = false;
    //     }
    //     // edge arrived too late
    //     else {
    //         recvErrorFlag = true;
    //         errorCode = EDGE_LATE;
    //     }
    // }
}


// Calculate the even parity bit for a 32 bit vector
uint32_t parity32(uint32_t x) {

    int i;
    uint32_t parity = 0UL;
  
    for (i = 0; i < 32; i++) {
        if (x & 1UL == 1UL) {
            parity = parity ^ 1UL;
        }
        x = x >> 1;
    }
    Serial.println(parity);
    return parity;
}


// Send a single machester encoded bit.
void sendMachesterBit(int val) {;
    digitalWrite(outPin, val);
    delayMicroseconds(500);
    digitalWrite(outPin, not val);
    delayMicroseconds(500);
}


void sendFrame(uint32_t msgType, uint32_t dataId, uint32_t dataValue) {

    char printBuffer[80];
    uint32_t mask = 0x80000000;

    // Create the message
    uint32_t msg = dataValue;
    msg |= dataId << 16;
    msg |= msgType << 28;
    msg |= parity32(msg) << 31;

    sprintf(printBuffer, "msg = 0x%08lx", msg);
    Serial.println(printBuffer);

    detachInterrupt(interruptNr);

    // Send start bit
    sendMachesterBit(true);

    // Send message
    while (mask != 0UL) {
        if (msg & mask) {
            sendMachesterBit(1);
        } else {
            sendMachesterBit(0);
        }
        mask = mask >> 1;
    }

    // Send stop bit
    sendMachesterBit(true);

    attachInterrupt(interruptNr, recvIsr, CHANGE);

}


void setup() {
  
    pinMode(inPin, INPUT);
    pinMode(outPin, OUTPUT);

    Serial.begin(115200);
    Serial.println("start");
    
    Serial.setTimeout(20);

    // set up WDT interrupt
    wdt_reset();
    WDTCSR |= (1 << WDCE) | (1 << WDE); // change enable
    WDTCSR = (1 << WDP0); // 64 ms
    attachInterrupt(interruptNr, recvIsr, CHANGE);
}


void loop() {

    int i;
    char printBuffer[80];
    uint32_t tRef;

//    Serial.println(parity32(0x0));
//    Serial.println(parity32(0x1));
//    Serial.println(parity32(0xF035CA0F));
//    Serial.println(parity32(0xE7D));

    // ask status
    // Serial.println("\nsend msg");
        
    // // ask boiler water temperature
    sendFrame(0UL, 25UL, 0UL);

    // Listen for a reply for 1s
    tRef = millis();
    while(millis() - tRef < 2000) {

        // if (recvError) {
        //     Serial.println("\nerror:"); 
        //     sprintf(printBuffer, "count = %d", recvCountBuffer);
        //     Serial.println(printBuffer);
        //     sprintf(printBuffer, "data = 0x%08x", recvDataBuffer);
        //     Serial.println(printBuffer);
        //     sprintf(printBuffer, "error code = %d", errorCodeBuffer);
        //     Serial.println(printBuffer);
        //     recvError = false;
        // } 
        
        // if (recvReady) {
        //     Serial.println("\nreceived message:"); 
        //     sprintf(printBuffer, "data = 0x%08x", recvDataBuffer);
        //     Serial.println(printBuffer);
        //     sprintf(printBuffer, "count = %d", recvCountBuffer);
        //     Serial.println(printBuffer); 
        //     recvReady = false;
        // }
    }
}
