#include <stdint.h>
#include <math.h>
#include <TimerOne.h>
#include <avr/wdt.h>

// message receive timeout;
ISR(WDT_vect){
    Serial.println("wdt");
}

void setup() {

    Serial.begin(115200);
    Serial.println("start");

    // set up WDT interrupt
    wdt_reset();
    WDTCSR |= (1 << WDCE) | (1 << WDE); // change enable
    WDTCSR = (1 << WDP2); // 64 ms
    WDTCSR |= (1<<WDIE); // Enable wdt interrupt
}


void loop() {

}