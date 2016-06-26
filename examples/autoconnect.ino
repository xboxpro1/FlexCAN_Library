// -------------------------------------------------------------
// FlexCan auto connect
// by xboxpro1
//

#include "FlexCAN.h"

String dataIN = "";
String crIN;
short CR = 10;
int led = 13;
int cbindex = 255;
boolean rx = false;
FlexCAN can;
static CAN_message_t rxmsg;


// -------------------------------------------------------------

void setup(void)
{
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  Serial.println("Teensy 3.2 CAN Auto Connect Test.");
  Serial.println("Press -> c <- for Auto Connect Test");
  Serial.println("Press -> r <- for Can RX Test");
 }


// -------------------------------------------------------------
void loop(void)
{
 if (Serial.available() > 0) {
   dataIN = Serial.readStringUntil(CR);
   crIN = Serial.readString();
    if(dataIN == 'c'){
        cbindex = can.connect();                // connect can bus default (0, 0) return speed index
        Serial.print("Speed Index -> ");
        Serial.println(cbindex);
        if(cbindex != 255){
          digitalWrite(led, 1);
        }
        else {
          digitalWrite(led, 0);
        }
    }
    
    else if(dataIN == 'r'){
      rx = true;
   }
    else{
      Serial.println("Press -> c <- for Auto Connect Test");
      Serial.println("Press -> r <- for Can RX Test");
      digitalWrite(led, 0);
      rx = false;
   }
 }
 if (rx == true && cbindex != 255){
    if(1 == can.available()){
      can.read(rxmsg);
      Serial.print("PID: ");
      Serial.print(rxmsg.id, HEX);
      Serial.print(" Data:");
      for (int x = 0; x < rxmsg.len; x ++){
      Serial.print(" ");
      if(rxmsg.req == 0){
        if(rxmsg.buf[x] < 10){
          Serial.print("0");
        }
        Serial.print(rxmsg.buf[x], HEX);
        }
      else {
        Serial.print("00");
        }
      }
      Serial.print(" Request: ");
      Serial.print(rxmsg.req);
      Serial.print(" Timestamp: ");
      Serial.println(rxmsg.timestamp);  
    }
 }
      
}

