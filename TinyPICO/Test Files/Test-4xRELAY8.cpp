// This code tests the RELAY8 shields ability to trigger the output relays / illuminate each of the LEDs.

#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_MCP23017.h>
Adafruit_MCP23017 outMCPs[4];
boolean relayState[32] = {false};
byte binValue[4] = {0,0,0,0};
int relayNumber[96] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};

/* ========================================================================= */

void relay(int inNum)
{

  // Map Relay
  int outNum = relayNumber[inNum - 1];
  if (!relayState[outNum])
  {
    relayState[outNum] = true;
  }
  else
  {
    relayState[outNum] = false;
  }

  // Set Relay
  int relayBoardNumber = ceil(((int)outNum - 1) / 8);
  byte relayPinPosition = outNum - (relayBoardNumber * 8);
  if (relayState[outNum])
  { 
    byte mask = 1 << (relayPinPosition - 1);
    binValue[relayBoardNumber] = binValue[relayBoardNumber] | mask;
  }
  else
  {
    byte mask = 255 - ( 1 << (relayPinPosition - 1));
    binValue[relayBoardNumber] = binValue[relayBoardNumber] & mask;
  }

  // Write Relay
  byte outHex[4] = { 0x20, 0x21, 0x22, 0x23 };             // Change if you are using different addresses for the RELAY8 shield than shown
  Wire.beginTransmission(outHex[relayBoardNumber]);
  Wire.write(0x12);
  Wire.write(binValue[relayBoardNumber]);
  Wire.endTransmission();

}

/* ========================================================================= */

void loop()
{

  for (unsigned char i = 1; i < 33; i++)
  { 
    relay(i);
    delay(200);
    relay(i);
  }
  delay(5000);                                             // Providing a 5 second delay so it doesn't just run over and over

}

/* ========================================================================= */

void setup()
{

  Serial.begin(9600);
  Wire.begin(33, 32, 100000);                             // TinyPICO likes to have the pins defined, whereas the Arduino doesn't seem to care as much
  for (unsigned char j = 0; j < 4; j++)                   // Adjust j < 4 according to how many RELAY8 shields you have on the Relay Shield Carrier 
  {
    outMCPs[j].begin(j, &Wire);
    for (unsigned char k = 0; k < 8; k++)
    {
      outMCPs[j].pinMode(k, OUTPUT);
      outMCPs[j].pullUp(k, HIGH);
    }
  } 

}

/* ========================================================================= */
