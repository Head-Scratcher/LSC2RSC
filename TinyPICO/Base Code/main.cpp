// This code is stripped down to just the bare essentials.
// It will take a button press from the Light Switch Controller, and turn on the matching relay on the Relay Shield Carrier.

#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_MCP23017.h>

#define DEBOUNCE_DURATION 25
#define SDA_1 21
#define SCL_1 22
#define SDA_2 33
#define SCL_2 32

TwoWire inBus = TwoWire(0); TwoWire outBus = TwoWire(1);
Adafruit_MCP23017 inMCPs[6]; Adafruit_MCP23017 outMCPs[4];
boolean relayState[32] = {false}; boolean FTT[96] = {true}; byte binValue[4] = {0,0,0,0}; boolean testRelay = true;
int relayNumber[96] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};

/* ========================================================================= */
  
   void relay(int inNum)
  {  
    // Map Relay
    int outNum = relayNumber[inNum - 1]; if (!relayState[outNum]) { relayState[outNum] = true; } else { relayState[outNum] = false; }

    // Set Relay
    int relayBoardNumber = ceil(((int)outNum - 1) / 8); byte relayPinPosition = outNum - (relayBoardNumber * 8);
    // Serial.print("Relay Pin Number (Linear) = "); Serial.println(outNum);
    // Serial.print("Relay Board Number = "); Serial.println(relayBoardNumber + 1);
    // Serial.print("Relay Board Pin Position = "); Serial.println(relayPinPosition);
    if (relayState[outNum]) { byte mask = 1 << (relayPinPosition - 1); binValue[relayBoardNumber] = binValue[relayBoardNumber] | mask; }
    else { byte mask = 255 - ( 1 << (relayPinPosition - 1)); binValue[relayBoardNumber] = binValue[relayBoardNumber] & mask; }
    // Serial.print("Binary Value = "); Serial.println(binValue[relayBoardNumber]);

    // Write Relay
    byte outHex[4] = { 0x20, 0x21, 0x22, 0x23 };
    outBus.beginTransmission(outHex[relayBoardNumber]);
    // Serial.print("Relay Board Address = "); Serial.println(outHex[relayBoardNumber]); 
    // Serial.println(" =================================== ");    
    outBus.write(0x12);
    outBus.write(binValue[relayBoardNumber]);
    outBus.endTransmission();
}

/* ========================================================================= */

void loop()  
{
  if (testRelay) { testRelay = false; for (unsigned char i = 0; i < 32; i++) { relayState[i] = false; } }
  
  int inNum = 1; unsigned long timeStart = 0;
  for (unsigned char i = 0; i < 6; i++) {
    for (unsigned char j = 0; j < 16; j++) {
      if (1 != inMCPs[i].digitalRead(j)) { 
        if (FTT[inNum]) { FTT[inNum] = false; timeStart = millis(); }
        if (millis()-timeStart > DEBOUNCE_DURATION) { timeStart = millis(); relay(inNum); while (1 != inMCPs[i].digitalRead(j)){} }
      }
    else { if (!FTT[inNum]) { FTT[inNum] = true;} }
    inNum++;
} } }

/* ========================================================================= */

void setup()
{
  Serial.begin(9600); inBus.begin(SDA_1, SCL_1, 100000); outBus.begin(SDA_2, SCL_2, 100000);

  for (unsigned char j = 0; j < 6; j++) { inMCPs[j].begin(j, &inBus); 
    for (unsigned char k = 0; k < 16; k++) { inMCPs[j].pinMode(k, INPUT); inMCPs[j].pullUp(k, HIGH); } }

  for (unsigned char j = 0; j < 4; j++) { outMCPs[j].begin(j, &outBus);
    for (unsigned char k = 0; k < 8; k++) { outMCPs[j].pinMode(k, OUTPUT); outMCPs[j].pullUp(k, HIGH); } } 
}

/* ========================================================================= */
