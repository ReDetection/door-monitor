/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#define DEBUGSERVER
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#ifdef  DEBUGSERVER

int serial_putc( char c, FILE * )  {
  Serial.write( c );
  return c;
} 

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}


#endif

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

#ifdef DEBUGSERVER
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
//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xE8E8F5FCA1LL, 0xE8E8F5F0E1LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

void setup(void)
{
    //
    // Print preamble
    //
#ifdef DEBUGSERVER
    
    Serial.begin(57600);
    printf_begin();
    printf("\n\rRF24/examples/GettingStarted/\n\r");
    printf("ROLE: %s\n\r",role_friendly_name[role]);
    printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
#endif
    //
    // Setup and configure rf radio
    //
    
    radio.begin();
    
    // optionally, increase the delay between retries & # of retries
    radio.setRetries(15,15);
    radio.setChannel(3);
    
    // optionally, reduce the payload size.  seems to
    // improve reliability
    radio.setPayloadSize(sizeof(Message));
    
    //
    // Open pipes to other nodes for communication
    //
    
    // This simple sketch opens two pipes for these two nodes to communicate
    // back and forth.
    // Open 'our' pipe for writing
    // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
    
    //if ( role == role_ping_out )
    {
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
    }
    //else
    {
        //radio.openWritingPipe(pipes[1]);
        //radio.openReadingPipe(1,pipes[0]);
    }
    
    //
    // Start listening
    //
    
    radio.startListening();
    
#ifdef DEBUGSERVER
    //
    // Dump the configuration of the rf unit for debugging
    //
    
    radio.printDetails();
#endif
}

void loop(void)
{
    //
    // Ping out role.  Repeatedly send the current time
    //
    
    if (role == role_ping_out)
    {
        // First, stop listening so we can talk.
        radio.stopListening();
        
        // Take the time, and send it.  This will block until complete
        unsigned long time = millis();
#ifdef DEBUGSERVER
        printf("Now sending %lu...",time);
#endif
        bool ok = radio.write( &time, sizeof(unsigned long) );
        
#ifdef DEBUGSERVER
        if (ok)
            printf("ok...");
        else
            printf("failed.\n\r");
#endif
        
        // Now, continue listening
        radio.startListening();
        
        // Wait here until we get a response, or timeout (250ms)
        unsigned long started_waiting_at = millis();
        bool timeout = false;
        while ( ! radio.available() && ! timeout )
            if (millis() - started_waiting_at > 200 )
                timeout = true;
        
        // Describe the results
        if ( timeout )
        {
#ifdef DEBUGSERVER
            printf("Failed, response timed out.\n\r");
#endif
        }
        else
        {
            // Grab the response, compare, and send to debugging spew
            unsigned long got_time;
            radio.read( &got_time, sizeof(unsigned long) );
            
#ifdef DEBUGSERVER
            // Spew it
            printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
#endif
        }
        
        // Try again 1s later
        delay(1000);
    }
    
    //
    // Pong back role.  Receive each packet, dump it out, and send it back
    //
    
    if ( role == role_pong_back )
    {
        // if there is data ready
        if ( radio.available() )
        {
            // Dump the payloads until we've gotten everything
            Message message;
//            bool done = false;
//            while (!done)
//            {
                // Fetch the payload, and see if this was the last one.
                radio.read( &message, sizeof(Message) );
                
#ifdef DEBUGSERVER
                // Spew it
                printf("Got payload %lu, door is %s...",message.time, message.doorIsClosed ? "closed": "opened");
#endif
                
                // Delay just a little bit to let the other unit
                // make the transition to receiver
                delay(40);
//            }
            
            // First, stop listening so we can talk
            radio.stopListening();
           

            // Send the final one back.
            bool ack = radio.write( &message, sizeof(Message) );
#ifdef DEBUGSERVER
            printf("Sent response, ack == %d.\n\r", ack ? 1 : 0);
            radio.printDetails();
#endif
            
            // Now, resume listening so we catch the next packets.
            radio.startListening();
        }
    }
    
    //
    // Change roles
    //
    
    if ( Serial.available() )
    {
        char c = toupper(Serial.read());
        if ( c == 'T' && role == role_pong_back )
        {
#ifdef DEBUGSERVER
            printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");
#endif
            
            // Become the primary transmitter (ping out)
            role = role_ping_out;
            radio.openWritingPipe(pipes[0]);
            radio.openReadingPipe(1,pipes[1]);
        }
        else if ( c == 'R' && role == role_ping_out )
        {
#ifdef DEBUGSERVER
            printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");
#endif
            
            // Become the primary receiver (pong back)
            role = role_pong_back;
            radio.openWritingPipe(pipes[1]);
            radio.openReadingPipe(1,pipes[0]);
        }
    }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
