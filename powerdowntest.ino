// **** INCLUDES *****
#include "LowPower.h"
#define LED_PIN 3

void wakeUp() {
      // Disable external pin interrupt on wake up pin.
//      detachInterrupt(0); 
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    
    // Configure wake up pin as input.
    // This will consumes few uA of current.
    pinMode(2, INPUT);  
    delay(20);
   // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(0, wakeUp, CHANGE);
     
}

void loop() 
{
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    
    
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);

}
