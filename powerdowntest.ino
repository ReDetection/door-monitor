#include "LowPower.h"

#define LED_PIN 3
#define DOOR_PIN 2

void wakeUp() {}

void sendState(bool open) {
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(10);
}

bool isDoorClosed() {
    return digitalRead(DOOR_PIN) == HIGH;
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
  
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);

    pinMode(DOOR_PIN, INPUT);
    delay(20);

    sendState(true);

    attachInterrupt(0, wakeUp, CHANGE);
}

void loop() {
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    delay(20);
    
    bool currentDoorState = isDoorClosed();
    bool doorStateToSend;
    do {
        doorStateToSend = currentDoorState;
        sendState(doorStateToSend);
        currentDoorState = isDoorClosed();
    } while (doorStateToSend != currentDoorState);
}
