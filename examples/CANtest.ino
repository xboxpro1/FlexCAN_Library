// -------------------------------------------------------------
// CANtest for Teensy 3.1
// by teachop
//
// This test is talking to a single other echo-node on the bus.
// 6 frames are transmitted and rx frames are counted.
// Tx and rx are done in a way to force some driver buffering.
// Serial is used to print the ongoing status.
//

#include "Metro.h"
#include "FlexCAN.h"

Metro sysTimer = Metro(1);// milliseconds

int led = 17;
FlexCAN CANbus;
static CAN_message_t rxmsg;
static uint8_t hex[17] = "0123456789abcdef";

unsigned int rxTimer;


// -------------------------------------------------------------
static void hexDump(uint8_t dumpLen, uint8_t *bytePtr)
{
  uint8_t working;
  while( dumpLen-- ) {
    working = *bytePtr++;
    Serial.write( hex[ working>>4 ] );
    Serial.write( hex[ working&15 ] );
  }
  Serial.write('\r');
  Serial.write('\n');
}


// -------------------------------------------------------------
void setup(void)
{
  CANbus.begin();
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  delay(1000);
  Serial.println(F("Hello Teensy 3.2 CAN Test."));

  sysTimer.reset();
  rxTimer = 3;                //milliseconds
}


// -------------------------------------------------------------
void loop(void)
{
  // service software timers based on Metro tick
  if ( sysTimer.check() ) {
    --rxTimer;
  }

  // if not time-delayed, read CAN messages and print
  if ( !rxTimer ) {
    while ( CANbus.read(rxmsg) ) {
      hexDump( sizeof(rxmsg), (uint8_t *)&rxmsg );
      //Serial.write(rxmsg.buf[0]);
      rxTimer = 3;    //milliseconds
    }
  }

  
}

