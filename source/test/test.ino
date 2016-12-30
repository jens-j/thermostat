#include <stdint.h>
#include <math.h>
#include <TimerOne.h>
#include <avr/wdt.h>

// IO pins
#define outPin 2
#define inPin 3
#define interruptNr 1 // interrupt number for pin 3
#define RECV_TIME_BUFFER 40

// Check if the time between X and Y in microseconds fall with bounds of T1 and T2
// Which are the half and whole period for the opentherm manchester encoding (0.5ms, 1ms)
#define CHECKT1(X, Y) ((Y-X) > 400 && (Y-X) < 650) 
#define CHECKT2(X, Y) ((Y-X) > 900 && (Y-X) < 1150)

// Variables related to receiving messages
volatile int recvCount = 0; // the number of received bits
volatile int recvCountBuffer;
volatile bool recvBusy = false; /// indicate a receive is in progress
volatile unsigned long recvData; // buffer to hold the received bits
volatile unsigned long recvDataBuffer;
volatile bool recvErrorFlag; // receive error flag
volatile char recvErrorCode[40]; // Error description
volatile unsigned long recvTimeRef; // timestamp of last clock transition
volatile bool recvHalf = false; // signal the last detected transition was a half period transition
volatile bool recvReady = false; // signal a message has been reveived
volatile bool recvError = false; // signal a receive error occured
volatile bool recvTimeout = false; // signal a receive timeout occured
volatile unsigned long recvTime[RECV_TIME_BUFFER];
volatile unsigned long recvTimeBuffer[RECV_TIME_BUFFER];




// message receive timeout;
ISR(WDT_vect){

    int i;
    
    // Disable wdt interrupt
    WDTCSR &= ~(1<<WDIE); 

    // Buffer data
    recvCountBuffer = recvCount;
    recvDataBuffer = recvData;
    for (i = 0; i < recvCount; i++) {
        recvTimeBuffer[i] = recvTime[i];
    }

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

    // bool checkT1;
    unsigned long t = micros();
    wdt_reset();
    recvCount++;

    if (recvBusy == false) {
        recvBusy = true;
        recvData = 0x00000000;
        recvCount = 0;
        WDTCSR |= (1<<WDIE); // Enable wdt interrupt
    } else {
        recvTime[recvCount-1] = t - recvTimeRef;
    }

    if (digitalRead(inPin) == LOW) {
        recvData |= 1UL << recvCount;
    } 

    recvTimeRef = t;


    // // If an error happened, discard all data and wait for a timeout
    // if (recvErrorFlag == true)
    //     return;

    // // Discard the first transition of the start bit and initialize variables
    // if (recvBusy == false) {
    //     recvBusy = true;
    //     recvCount = 0;
    //     recvHalf = false;
    //     recvData = 0x00000000;
    //     WDTCSR |= (1<<WDIE); // Enable wdt interrupt
    // } 
    // // First clock transition of the start bit
    // else if (recvCount == 0) {
    //     // if (!CHECKT1(recvTimeRef, t)) {
    //     //     recvErrorFlag = true;
    //     //     sprintf(recvErrorCode, "start bit timing (%lu)", t - recvTimeRef);
    //     // }
    // } 
    // // All other transitions
    // else {
    //     checkT1 = CHECKT1(recvTimeRef, t);
    //     // First half period transitions
    //     if (recvHalf == false && checkT1) {
    //         recvHalf = true;
    //     } 
    //     // Second half period and full period transitions
    //     else { //if (recvHalf == true  && checkT1 || 
    //            //  recvHalf == false && CHECKT2(recvTimeRef, t)) {
    //         // End of frame 
    //         if (recvCount++ == 32) {
    //             // Check stop bit 
    //             // if (!digitalRead(inPin)) {
    //                 WDTCSR &= ~(1<<WDIE); // Disable wdt interrupt
    //                 recvReady = true;
    //             // } else {
    //             //     recvErrorFlag = true;
    //             //     sprintf(recvErrorCode, "stop bit missing");
    //             // }
    //         } 
    //         // Store data bit
    //         else {
    //             recvData = recvData << 1;
    //             recvData |= ~digitalRead(inPin);
    //             recvHalf = false;
    //         }
    //     } 
    //     // Timing error
    //     // else {
    //     //     recvErrorFlag = true; 
    //     //     sprintf(recvErrorCode, "timing (%lu)", t - recvTimeRef);
    //     // }
    // }

    // recvTimeRef = t;
}


// Calculate the even parity bit for a 32 bit vector
int parity32(unsigned long x) {

    int i;
    int parity = 0;
  
    for (i = 0; i < 32; i++) {
        if (x & 1 == 1) {
            parity = parity ^ 1;
        }
        x = x >> 1;
    }
    return parity;
}


// Send a single machester encoded bit.
void sendMachesterBit(int val) {;
    digitalWrite(outPin, val);
    delayMicroseconds(500);
    digitalWrite(outPin, not val);
    delayMicroseconds(500);
}


void sendFrame(uint8_t msgType, uint8_t dataId, uint16_t dataValue) {

    unsigned long mask = 0x80000000;

    // Create the message
    unsigned long msg = dataValue;
    msg |= dataId << 16;
    msg |= msgType << 28;
    msg |= parity32(msg) << 31;

    // Send start bit
    sendMachesterBit(true);

    // Send message
    while (mask != 0UL) {
        //Serial.println("b");
        sendMachesterBit(msg & mask);
        mask = mask >> 1;
    }

    // Send stop bit
    sendMachesterBit(true);
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
    unsigned long tRef;

//    Serial.println(parity32(0x0));
//    Serial.println(parity32(0x1));
//    Serial.println(parity32(0xF035CA0F));
//    Serial.println(parity32(0xE7D));

    // ask status
    Serial.println("\nsend msg");
    sendFrame(0, 0, 0);
        
    // // ask boiler water temperature
    // sendFrame(0, 25, 0);

    // Listen for a reply for 1s
    tRef = millis();
    while(millis() - tRef < 5000) {

        if (recvError) {
            Serial.println("\nerror:"); 
            sprintf(printBuffer, "count = %d", recvCountBuffer);
            Serial.println(printBuffer);
            sprintf(printBuffer, "0x%08x", recvDataBuffer);
            Serial.println(printBuffer);
            recvError = false;
        } 
        else if (recvTimeout) {
            recvTimeout = false;
            Serial.println("\ntimeout");
            sprintf(printBuffer, "count = %d", recvCountBuffer);
            Serial.println(printBuffer);
            sprintf(printBuffer, "0x%08x", recvDataBuffer);
            Serial.println(printBuffer);
            // for (i = 0; i < recvCountBuffer; i++) {
            //     sprintf(printBuffer, "%u ", (unsigned long) recvTimeBuffer[i]);
            //     Serial.print(printBuffer);
            // }
            // Serial.println("");
        } 
        else if (recvReady) {
            recvReady = false;
            Serial.println("\nreceived message:"); 
            sprintf(printBuffer, "type = %u, id = %x, data = %x", 
                    recvDataBuffer & 0x70000000, 
                    recvDataBuffer & 0x00ff0000,
                    recvDataBuffer & 0x0000ffff);
            Serial.println(printBuffer); 
        }
    }
}
