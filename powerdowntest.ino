#include "LowPower.h"

#define LED_PIN 3

void wakeUp() {}

void setup() {
    pinMode(LED_PIN, OUTPUT);
  
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);

    pinMode(2, INPUT);  
    delay(20);

    attachInterrupt(0, wakeUp, CHANGE);
}

void loop() {
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);


    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);

}
