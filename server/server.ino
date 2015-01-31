
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

int serial_putc( char c, FILE * )  {
    Serial.write( c );
    return c;
} 

void printf_begin(void) {
    fdevopen( &serial_putc, 0 );
}

#define PRO_MICRO_BOARD

#ifdef PRO_MICRO_BOARD
#define LED_PIN 2
RF24 radio(7,8);
#else
RF24 radio(9,10);
#endif

struct sMessage {
    unsigned long time;
    uint8_t doorIsClosed : 1;
    uint8_t doorIsLocked : 1;
    uint8_t              : 6;
};
typedef struct sMessage Message;

const uint64_t pipes[2] = { 0xE8E8F5FCA1LL, 0xE8E8F5F0E1LL };


void setup(void) {
    
    Serial.begin(57600);
#ifdef PRO_MICRO_BOARD
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(2000);
#endif
    printf_begin();


    radio.begin();
    radio.setRetries(15,15);
    radio.setChannel(3);
    radio.setPayloadSize(sizeof(Message));
    
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    radio.startListening();

//    radio.printDetails();
// TODO check if radio is connected and everything is OK
}

void loop(void) {
    
    while ( radio.available() ) {
        Message message;
        radio.read( &message, sizeof(Message) );
            
        printf("Got payload %lu, door is %s and %s... \n",message.time, message.doorIsClosed ? "closed": "opened", message.doorIsLocked ? "locked" : "unlocked");
    }

    delay(50);
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
