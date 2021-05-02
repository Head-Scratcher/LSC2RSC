// Version 1.0 - Connnects the TinyPICO to a RaspberryPi4B over the GPIO pins to do serial communication. Note: Port and pin settings may vary between models. 
// Requires RPi Version 1.0.py to be installed on the RPi to get a response. If the script isn't running on the RPi, it will use a local copy of the relay settings.
// SSH into the RPi to see what is being received (may need to uncomment line in RPi Version 1.0.py to see results).
// Libraries

#include <Wire.h>                                                    // For I2C
#include <Arduino.h>
#include <Ethernet.h>                                                // For networking
#include "LSC_Button.h"                                              // For button click handling
#include "Adafruit_MCP23017.h"                                       // For MCP23017 I/O buffers
// Constants
#define RXS0 4                                                       // dev/ttyUSB0 (laptop via the TP USB-C port)
#define TXS0 5                                                       // dev/ttyUSB0 (laptop via the TP USB-C port)
#define RXS1 18                                                      // dev/ttyS0 over on RPi via RX-TX pins
#define TXS1 19                                                      // dev/ttyS0 over on RPi via RX-TX pins
#define SDA_1 21                                                     // SDA on LSC
#define SCL_1 22                                                     // SCL on LSC
#define SDA_2 33                                                     // SDA on RSC
#define SCL_2 32                                                     // SCL on RSC
const byte IN_MCP_ADDRESS[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25 };
const byte OUT_MCP_ADDRESS[] = { 0x20, 0x21, 0x22, 0x23 };
// Global Variables
uint8_t in_mcps_found = 0;                                           // Each bit corresponds to an MCP found on the IC2 bus
uint8_t out_mcps_found = 0;                                          // Each bit corresponds to an MCP found on the IC2 bus
boolean relayState[32];
boolean buttonState[96];
boolean setRelayState = true;
byte binValue[4] = {0,0,0,0};
// Instantiate Global Objects
LSC_Button button[sizeof(IN_MCP_ADDRESS)];                           // Button handlers
Adafruit_MCP23017 IN_MCP23017[sizeof(IN_MCP_ADDRESS)];               // I/O in buffers
Adafruit_MCP23017 OUT_MCP23017[sizeof(OUT_MCP_ADDRESS)];             // I/O out buffers
// Callback Signatures
void buttonPressed(uint8_t id, uint8_t button, uint8_t state);

// ========================== Program ============================== //

void relayActions(int outNum) {

  if (!relayState[outNum]) {                                         // Update relayState
    relayState[outNum] = true; 
  }
  else {
    relayState[outNum] = false;
  }

  int relayBoardNumber = ceil(((int)outNum - 1) / 8);                // Map Button to Relay
  byte relayPinPosition = outNum - (relayBoardNumber * 8);

  if (relayState[outNum]) { byte mask = 1 << (relayPinPosition - 1); // Set Binary Value
    binValue[relayBoardNumber] = binValue[relayBoardNumber] | mask; 
  }
  else {
    byte mask = 255 - ( 1 << (relayPinPosition - 1));
    binValue[relayBoardNumber] = binValue[relayBoardNumber] & mask;
  }
  
  Wire1.beginTransmission(OUT_MCP_ADDRESS[relayBoardNumber]);        // Write Relay
  Wire1.write(0x12);
  Wire1.write(binValue[relayBoardNumber]);  
  Wire1.endTransmission();
}

// ================================================================= //

void decidePath(unsigned char inNum) {

  Serial1.print(inNum); 
  int outNum = Serial1.parseInt();                                   // DEBUG:
  Serial.print(inNum); Serial.print("-"); Serial.println(outNum);
  
  if (outNum >0 && outNum < 33) { 
    relayActions(outNum); 
  }
  else { 
    int relayNumber[96] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,3,5,7,9,11,13,15,17,19,21,23,
    25,27,29,31,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,1,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2};
    relayActions(relayNumber[inNum- 1]); 
  }

}

// ================================================================= //

char * getMqttButtonAction(uint8_t state) {                          // Determine what action we need to publish

  static char action[7];
  switch (state) {
    case BUTTON_HOLD_STATE:
      sprintf_P(action, PSTR("HOLD")); break;
    case 1:
      sprintf_P(action, PSTR("SINGLE")); break;
    case 2:
      sprintf_P(action, PSTR("DOUBLE")); break;
    case 3:
      sprintf_P(action, PSTR("TRIPLE")); break;
    case 4:
      sprintf_P(action, PSTR("QUAD")); break;
    case 5:
      sprintf_P(action, PSTR("PENTA")); break;
    default:
      sprintf_P(action, PSTR("ERROR")); break;
  }
  return action;
}

// ================================================================= //

void buttonPressed(uint8_t id, uint8_t button, uint8_t state) {

  Serial.println(getMqttButtonAction(state));
  decidePath(button +1);

}

// ================================================================= //

void loop() {

  if (setRelayState) {
    setRelayState = false;
    for (unsigned char i = 0; i < 32; i++) {
      relayState[i] = false;
    }
  }

  for (uint8_t i = 0; i < sizeof(IN_MCP_ADDRESS); i++)               // Iterate through each of the MCP23017 input buffers
  {
    if (bitRead(in_mcps_found, i) == 0)
    continue;
    uint16_t io_value = IN_MCP23017[i].readGPIOAB();
    button[i].process(i, io_value);
  }
}

// ================================================================= //

void setup() {
  //                                                                 The TP has 3 physical UARTS 0,1 & 2, typically USB is going to want to use UART0. Shown as /dev/ttyUSB0
  Serial.begin(9600, SERIAL_8N1, RXS0, TXS0);                        // RXS0 GPIO  5(RX-TP) connected to LAPTOP via USB  ||| TXS0 GPIO  4(TX-TP) connected to LAPTOP via USB
  Serial1.begin(9600, SERIAL_8N1, RXS1, TXS1);                       // RXS1 GPIO 18(RX-TP) connected to GPIO 14(TX-RPi) ||| TXS1 GPIO 19(TX-TP) connected to GPIO 15(RX-RPi)
  Wire.begin(SDA_1, SCL_1, 100000); Wire1.begin(SDA_2, SCL_2, 100000);

  Serial.println(F(" In MCS23017s:"));
  for (uint8_t i = 0; i < sizeof(IN_MCP_ADDRESS); i++) {             // Scan the I2C bus for any MCP23017s and initialise them  
    Serial.print(F(" - 0x")); Serial.print(IN_MCP_ADDRESS[i], HEX); Serial.print(F("..."));
    Wire.beginTransmission(IN_MCP_ADDRESS[i]);                       // Set the bit indicating we found an MCP at this address
    bitWrite(in_mcps_found, i, Wire.endTransmission() == 0);
    if (bitRead(in_mcps_found, i)) {                                 // If a chip was found then initialise and configure the inputs
      IN_MCP23017[i].begin(i, &Wire);
      for (uint8_t pin = 0; pin < 16; pin++) {
        IN_MCP23017[i].pinMode(pin, INPUT);
        IN_MCP23017[i].pullUp(pin, HIGH);
      }
      button[i].onButtonPressed(buttonPressed);                      // Makes a call during setup?
      Serial.println(F("ok"));
    }
    else {
      Serial.println(F("Not ok")); 
    }
  }

  Serial.println(F(" Out MCS23017s:"));
  for (uint8_t i = 0; i < sizeof(OUT_MCP_ADDRESS); i++) {            // Scan the I2C bus for any MCP23017s and initialise them
    Serial.print(F(" - 0x")); Serial.print(OUT_MCP_ADDRESS[i], HEX); Serial.print(F("..."));
    Wire1.beginTransmission(OUT_MCP_ADDRESS[i]);                     // Set the bit indicating we found an MCP at this address
    bitWrite(out_mcps_found, i, Wire1.endTransmission() == 0);
    if (bitRead(out_mcps_found, i)) {                                                        // If a chip was found then initialise and configure the inputs
      OUT_MCP23017[i].begin(i, &Wire1);
      for (uint8_t pin = 0; pin < 8; pin++) {
        OUT_MCP23017[i].pinMode(pin, OUTPUT);
        OUT_MCP23017[i].pullUp(pin, HIGH);
      }
      Serial.println(F("ok"));
    }
    else {
      Serial.println(F("Not ok")); 
    }
  }
}

// ================================================================= //
