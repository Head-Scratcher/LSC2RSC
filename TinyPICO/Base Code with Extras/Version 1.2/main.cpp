// Version 1.2 - Connnects the TinyPICO to a RaspberryPi4B over the GPIO pins to do serial communication. Note: Port and pin settings may vary between models. 
// Requires RPi.py to be installed on the RPi to get a response. If the script isn't running on the RPi, it will use a local copy of the relay settings.
// SSH into the RPi to see what is being received (may need to uncomment line in RPi.py to see results).

#include <Arduino.h>                                                                          // LIBRARIES
#include <Wire.h>                                                                             // For I2C
#include "LSC_Button.h"                                                                       // For button click handling
#include "Adafruit_MCP23017.h"                                                                // For MCP23017 I/O buffers
#include <LiquidCrystal_I2C.h>
#define RXS0 4                                                                                // CONSTANTS                                                    
#define TXS0 5                                                                                // dev/ttyUSB0 (laptop via the TP USB-C port)
#define RXS1 18                                                                               // dev/ttyS0 over on RPi via RX-TX pins
#define TXS1 19                                                                               // dev/ttyS0 over on RPi via RX-TX pins
#define SDA_1 21                                                                              // SDA on LSC
#define SCL_1 22                                                                              // SCL on LSC
#define SDA_2 33                                                                              // SDA on RSC
#define SCL_2 32                                                                              // SCL on RSC
LiquidCrystal_I2C LCD_ADDRESS(0x27, 20, 4, LCD_5x10DOTS);
const byte OUT_MCP_ADDRESS[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26 };
const byte IN_MCP_ADDRESS[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
const int relayNumber[96] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,1,3,5,7,9,11,13,15,17,19,21,23,
25,27,29,31,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,1,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2};
Adafruit_MCP23017 IN_MCP23017[sizeof(IN_MCP_ADDRESS)];                                        // INSTANTIATE GLOBAL
Adafruit_MCP23017 OUT_MCP23017[sizeof(OUT_MCP_ADDRESS)];                                      // I/O out buffers
LSC_Button A_BUTTON[sizeof(IN_MCP_ADDRESS)];                                                  // Button handlers
byte binValue[4] = {0,0,0,0};                                                                 // GLOBAL VARIABLES
uint8_t in_mcps_found = 0;                                                                    // Each bit corresponds to an MCP found on the IC2 bus
boolean relayState[32];

// ========================================================================================== //

void buttonPressed(uint8_t mcpNum, uint8_t btnNum, uint8_t btnState) {

  boolean showScreen = false; boolean gotNum = false;
  switch (btnState) { case BUTTON_HOLD_STATE: showScreen = true; }
  byte inNum = (mcpNum * 16) + (btnNum + 1);
  Serial1.print(inNum); int outNum = Serial1.parseInt();

  if (outNum != 0 && outNum < 33) { gotNum = true; } else { showScreen = true; outNum = relayNumber[inNum -1]; }
  if (!relayState[outNum]) { relayState[outNum] = true; } else { relayState[outNum] = false; }                                           // Update relayState
  int relayBoardNumber = ceil(((int)outNum - 1) / 8);   byte relayPinPosition = outNum - (relayBoardNumber * 8);                         // Map Button to Relay
  if (relayState[outNum]) { byte mask = 1 << (relayPinPosition - 1); binValue[relayBoardNumber] = binValue[relayBoardNumber] | mask; }   // Set Binary Value
  else { byte mask = 255 - ( 1 << (relayPinPosition - 1)); binValue[relayBoardNumber] = binValue[relayBoardNumber] & mask; }
  Wire.beginTransmission(OUT_MCP_ADDRESS[relayBoardNumber]); Wire.write(0x12); Wire.write(binValue[relayBoardNumber]); Wire.endTransmission();

  if (showScreen && !gotNum) {
      LCD_ADDRESS.setCursor(0,0); LCD_ADDRESS.print(" CONNECTION  FAILED ");
      LCD_ADDRESS.setCursor(0,1); LCD_ADDRESS.print("    Using local:    ");
      LCD_ADDRESS.setCursor(0,2); LCD_ADDRESS.print("    Relay No:       ");
      LCD_ADDRESS.setCursor(0,3); LCD_ADDRESS.print("  Rebooting RPi...  ");
      LCD_ADDRESS.setCursor(14, 2); LCD_ADDRESS.print(outNum);
      LCD_ADDRESS.backlight(); delay(2500); LCD_ADDRESS.noBacklight(); LCD_ADDRESS.clear();
  }

  if (showScreen) { showScreen = false;
    LCD_ADDRESS.setCursor(0,0); LCD_ADDRESS.print("Button:   Relay:   ");
    LCD_ADDRESS.setCursor(7,0); LCD_ADDRESS.print(inNum);
    LCD_ADDRESS.setCursor(16,0); LCD_ADDRESS.print(outNum);
    LCD_ADDRESS.setCursor(0,1); LCD_ADDRESS.print("BoardNo:  PinPos:  ");
    LCD_ADDRESS.setCursor(8,1); LCD_ADDRESS.print(relayBoardNumber +1);
    LCD_ADDRESS.setCursor(17,1); LCD_ADDRESS.print(relayPinPosition);
    LCD_ADDRESS.setCursor(0,2); LCD_ADDRESS.print("State:    BinVal:  ");
    LCD_ADDRESS.setCursor(6,2); LCD_ADDRESS.print(relayState[outNum]); 
    LCD_ADDRESS.setCursor(17,2); LCD_ADDRESS.print(binValue[relayBoardNumber]);
    LCD_ADDRESS.setCursor(0,3); LCD_ADDRESS.print("Decimal   Address: ");
    LCD_ADDRESS.setCursor(18,3); LCD_ADDRESS.print(OUT_MCP_ADDRESS[relayBoardNumber]);
    LCD_ADDRESS.backlight(); delay(2500); LCD_ADDRESS.noBacklight(); LCD_ADDRESS.clear(); 
  }
}

// ========================================================================================== //

void loop() {

  for (uint8_t i = 0; i < sizeof(IN_MCP_ADDRESS); i++) {                                      // Iterate through each of the MCP23017 input buffers
    if (bitRead(in_mcps_found, i) == 0) continue;
    uint16_t io_value = IN_MCP23017[i].readGPIOAB();
    A_BUTTON[i].process(i, io_value);
  }
}

// ========================================================================================== //

void setup() {
  //                                                                 The TP has 3 physical UARTS 0,1 & 2, typically USB is going to want to use UART0. Shown as /dev/ttyUSB0
  Serial.begin(9600, SERIAL_8N1, RXS0, TXS0);                        // RXS0 GPIO  5(RX-TP) connected to LAPTOP via USB  ||| TXS0 GPIO  4(TX-TP) connected to LAPTOP via USB
  Serial1.begin(9600, SERIAL_8N1, RXS1, TXS1);                       // RXS1 GPIO 18(RX-TP) connected to GPIO 14(TX-RPi) ||| TXS1 GPIO 19(TX-TP) connected to GPIO 15(RX-RPi)
  Wire.begin(SDA_1, SCL_1, 100000); 
  Wire1.begin(SDA_2, SCL_2, 100000);
  boolean screenFound = false; uint8_t out_mcps_found = 0;
  for (unsigned char i = 0; i < 32; i++) { relayState[i] = false; }

  Wire.beginTransmission(0x27); bitWrite( out_mcps_found, 7, Wire.endTransmission() == 0);
  if (bitRead(out_mcps_found, 7)) { screenFound = true;
    LCD_ADDRESS.begin(); LCD_ADDRESS.backlight();
    // LCD_ADDRESS.setCursor(0,0); LCD_ADDRESS.print("<------------------>");
    // LCD_ADDRESS.setCursor(0,1); LCD_ADDRESS.print("|  Swanky Startup  |");
    // LCD_ADDRESS.setCursor(0,2); LCD_ADDRESS.print("|  Message v1.01   |");
    // LCD_ADDRESS.setCursor(0,3); LCD_ADDRESS.print("<------------------>");
    // delay(1500);
    // LCD_ADDRESS.clear();
    
    LCD_ADDRESS.setCursor(0,0); LCD_ADDRESS.print("  I2C Address 0x    ");
    LCD_ADDRESS.setCursor(0,2); LCD_ADDRESS.print("    IN:             ");
    LCD_ADDRESS.setCursor(0,3); LCD_ADDRESS.print("   OUT:             ");
  }

  byte _pos = 8; byte _value = 0;
  for (uint8_t i = 0; i < 8; i++) { 
    Wire1.beginTransmission(IN_MCP_ADDRESS[i]); 
    bitWrite(in_mcps_found, i, Wire1.endTransmission() == 0);
    if (bitRead(in_mcps_found, i)) { IN_MCP23017[i].begin(i, &Wire1);
      for (uint8_t pin = 0; pin < 16; pin++) { IN_MCP23017[i].pinMode(pin, INPUT); IN_MCP23017[i].pullUp(pin, HIGH); }
      A_BUTTON[i].onButtonPressed(buttonPressed); _value = 255; } else { _value = 88; } _pos++;
    if (screenFound) { LCD_ADDRESS.setCursor(16,0); LCD_ADDRESS.print(IN_MCP_ADDRESS[i], HEX); LCD_ADDRESS.setCursor(_pos, 2); LCD_ADDRESS.write(_value); delay(400); }
  }

  _pos = 8;
  for (uint8_t i = 0; i < 7; i++) {
    Wire.beginTransmission(OUT_MCP_ADDRESS[i]);
    bitWrite(out_mcps_found, i, Wire.endTransmission() == 0);
    if (bitRead(out_mcps_found, i)) { OUT_MCP23017[i].begin(i, &Wire);
      for (uint8_t pin = 0; pin < 8; pin++) { OUT_MCP23017[i].pinMode(pin, OUTPUT); OUT_MCP23017[i].pullUp(pin, HIGH); }
      _value = 255; } else { _value = 88; } _pos++;
    if (screenFound) { LCD_ADDRESS.setCursor(16,0); LCD_ADDRESS.print(OUT_MCP_ADDRESS[i], HEX); LCD_ADDRESS.setCursor(_pos, 3); LCD_ADDRESS.write(_value); delay(400); }
  }

  if (screenFound) { 
    LCD_ADDRESS.setCursor(16,0); LCD_ADDRESS.print(27);
    LCD_ADDRESS.setCursor(_pos +1, 3); LCD_ADDRESS.write(255);    
    delay(2000); LCD_ADDRESS.noBacklight(); LCD_ADDRESS.clear();    
  }
}

// ========================================================================================== //
