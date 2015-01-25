#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "LowPower.h"

//#define DEBUGSERVER
#ifdef  DEBUGSERVER

int serial_putc( char c, FILE * )  {
  Serial.write( c );
  return c;
} 

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}


#endif

#define LED_PIN 3
#define DOOR_PIN 2


#ifdef DEBUGSERVER
 RF24 radio(7,8);
#else
 RF24 radio(9,10);
#endif

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
    Message messageAck;
    message.time = millis();
    message.doorIsClosed = closed ? 1 : 0;
    message.doorIsLocked = 0;
#ifdef DEBUGSERVER
    printf("Constructed message with time %lu and flag %d\n\r", message.time, message.doorIsClosed);
#endif

    bool success = false;
    while (!success) {
#ifdef DEBUGSERVER
      printf("sending... ");
#endif
      success = radio.write(&message, sizeof(Message));
    
#ifdef DEBUGSERVER
      printf("ack == %d\n", success? 1 : 0);
#endif
    }
    
    digitalWrite(LED_PIN, LOW);
    delay(10);
}

bool isDoorClosed() {
    return digitalRead(DOOR_PIN) == HIGH;
}

void setup() {
#ifdef DEBUGSERVER

    Serial.begin(57600);
    printf_begin();
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(2000);
    

    printf("\n\rNODE\n\r");
    printf("ulong: %d, message: %d\n\r", sizeof(unsigned long), sizeof(Message));
    
#endif


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
    radio.setRetries(15,15);
    radio.openWritingPipe(dstAddress);
    radio.openReadingPipe(1,nodeAddress);
    radio.setChannel(3);
    radio.setPayloadSize(sizeof(Message));

#ifdef DEBUGSERVER
    radio.printDetails();
#endif
    sendState(isDoorClosed());

    attachInterrupt(0, wakeUp, CHANGE);
}

void loop() {
    radio.powerDown();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    radio.powerUp();
    delay(20);
    
    bool currentDoorState = isDoorClosed();
    bool doorStateToSend;
    do {
        doorStateToSend = currentDoorState;
        sendState(doorStateToSend);
        currentDoorState = isDoorClosed();
    } while (doorStateToSend != currentDoorState);
}
