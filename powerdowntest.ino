#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "LowPower.h"

#define LED_PIN 3
#define DOOR_PIN 2

RF24 radio(9,10);
const uint64_t dstAddress = 0xE8E8F5F0E1LL;
const uint64_t nodeAddress = 0xE8E8F5FCA1LL;

struct sMessage {
  unsigned long time;
  uint8_t doorIsClosed : 1;
  uint8_t doorIsLocked : 1;
  uint8_t              : 6;
};
typedef struct sMessage Message;

void wakeUp() {}

void sendState(bool closed) {
    digitalWrite(LED_PIN, HIGH);
    delay(10);
    
    Message message;
    message.time = millis();
    message.doorIsClosed = closed ? 1 : 0;
    message.doorIsLocked = 0;
    
    bool success = false;
    while (!success) {
      radio.write(&message, sizeof(Message));
    
      unsigned long started_waiting_at = millis();
      bool timeout = false;
      while ( ! radio.available() && ! timeout )
        if (millis() - started_waiting_at > 100 )
          timeout = true;

      if (timeout) {
        printf("Failed, response timed out.\n\r");
      
      } else {
        unsigned long ackMessageWithTime;
        radio.read( &ackMessageWithTime, sizeof(unsigned long) );
        success = ackMessageWithTime == message.time;
      }
    }
    
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
    
    radio.begin();
    radio.openWritingPipe(dstAddress);
    radio.openReadingPipe(1,nodeAddress);

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
