// Version 1.0 - Connnects the TinyPICO to a RaspberryPi4B over the GPIO pins to do serial communication. Note: Port and pin settings may vary between models. 
// Requires RPi Version 1.0.py to be installed on the RPi to get a response. If the script isn't running on the RPi, it will use a local copy of the relay settings.
// SSH into the RPi to see what is being received (may need to uncomment line in RPi Version 1.0.py to see results).

#include <Adafruit_MCP23017.h>
#include <Arduino.h>
#include <Wire.h>
#define RXS0 4                                   // dev/ttyUSB0 (laptop via the TP USB-C port)
#define TXS0 5                                   // dev/ttyUSB0 (laptop via the TP USB-C port)
#define RXS1 18                                  // dev/ttyS0 over on RPi via RX-TX pins
#define TXS1 19                                  // dev/ttyS0 over on RPi via RX-TX pins
#define SDA_1 21                                 // SDA on LSC
#define SCL_1 22                                 // SCL on LSC
#define SDA_2 33                                 // SDA on RSC
#define SCL_2 32                                 // SCL on RSC
#define DEBOUNCE_DURATION 25
Adafruit_MCP23017 inMCPs[6]; Adafruit_MCP23017 outMCPs[4];
boolean relayState[32]; boolean buttonState[96]; boolean setRelay = true;
byte binValue[4] = {0,0,0,0}; unsigned long timeStart = 0;

/* ========================================================================= */

void relayActions(int outNum) {
  // Update relayState
  if (!relayState[outNum]) { relayState[outNum] = true; } else { relayState[outNum] = false; }

  // Map Button to Relay
  int relayBoardNumber = ceil(((int)outNum - 1) / 8); byte relayPinPosition = outNum - (relayBoardNumber * 8);

  // Set Binary Value
  if (relayState[outNum]) { byte mask = 1 << (relayPinPosition - 1); binValue[relayBoardNumber] = binValue[relayBoardNumber] | mask; }
  else { byte mask = 255 - ( 1 << (relayPinPosition - 1)); binValue[relayBoardNumber] = binValue[relayBoardNumber] & mask; }
    
  // Write Relay
  byte outHex[4] = { 0x20, 0x21, 0x22, 0x23 };
  Wire1.beginTransmission(outHex[relayBoardNumber]); 
  Wire1.write(0x12);
  Wire1.write(binValue[relayBoardNumber]);  
  Wire1.endTransmission();
}

/* ========================================================================= */

void decidePath(unsigned char inNum) {

  Serial1.print(inNum); 
  int outNum = Serial1.parseInt();                          // DEBUG: Serial.print(inNum); Serial.print("-"); Serial.println(outNum);
  if (outNum >0 && outNum < 33) { relayActions(outNum); }
  else { 
  int relayNumber[96] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,3,5,7,9,11,13,15,17,19,21,23,
  25,27,29,31,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,1,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2};
  relayActions(relayNumber[inNum- 1]); }

}

/* ========================================================================= */
void loop() {

  if (setRelay) { setRelay = false; for (unsigned char i = 0; i < 32; i++) { relayState[i] = false; } }

  unsigned char inNum = 0;                                  // Button number / index
  for (unsigned char i = 0; i < 6; i++) {                   // Outisde loop 0 to 5 (6)
    for (unsigned char j = 0; j < 16; j++) {                // Inside loop 0 to 15 (16)  
      if (0 == inMCPs[i].digitalRead(j)) {                  // Reading the actual state of the MCP23017 pin, 1 when not pressed, 0 when pressed... 
        if (!buttonState[inNum]) {                          // Looking to RETURN & USE a true / false. If the VALUE of buttonState[inNum] = false, then RETURN & USE true 
          buttonState[inNum] = true;                        // Setting to true to stop further work being done
          decidePath(inNum +1);                             // Go do work son...
        }                                                   // Most Infinitesimal chance the button was release during this cycle, but so small not worth checking
      timeStart = millis();                                 // timeStart has to be set inside this if at the last moment before checking debounce
      }                                                     // Most Infinitesimal chance the button was release during an else condition cycle, so small not worth checking     
      else {
        if (millis()-timeStart > DEBOUNCE_DURATION) {       // Checking debounce now the button has been released. Can't debounce the button until the button is released 
          if (buttonState[inNum]) {
             buttonState[inNum] = false; 
      } } }
    inNum++;                                                // Increment the main button number / index by 1
} } }

/* ========================================================================= */

void setup() {
  //                                                           The TP has 3 physical UARTS 0,1 & 2, typically USB is going to want to use UART0. Shown as /dev/ttyUSB0
  Serial.begin(9600, SERIAL_8N1, RXS0, TXS0);               // RXS0 GPIO  5(RX-TP) connected to LAPTOP via USB  ||| TXS0 GPIO  4(TX-TP) connected to LAPTOP via USB
  Serial1.begin(9600, SERIAL_8N1, RXS1, TXS1);              // RXS1 GPIO 18(RX-TP) connected to GPIO 14(TX-RPi) ||| TXS1 GPIO 19(TX-TP) connected to GPIO 15(RX-RPi)      
  
  Wire.begin(SDA_1, SCL_1, 100000);
  Wire1.begin(SDA_2, SCL_2, 100000);

  for (unsigned char j = 0; j < 6; j++) {
    inMCPs[j].begin(j, &Wire);
    for (unsigned char k = 0; k < 16; k++) {
      inMCPs[j].pinMode(k, INPUT);
      inMCPs[j].pullUp(k, HIGH);
  } }

  for (unsigned char j = 0; j < 4; j++) {
    outMCPs[j].begin(j, &Wire1);
    for (unsigned char k = 0; k < 8; k++) {
      outMCPs[j].pinMode(k, OUTPUT);
      outMCPs[j].pullUp(k, HIGH);
} } }

/* ========================================================================= */
