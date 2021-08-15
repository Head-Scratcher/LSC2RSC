// Configuration & Libraries -------------------------------------------------------------------------------------------------------------------------

#include "config.h"                                                         // Should be no user configuration in this file, everything should be in;
#include <Wire.h>                                                           // For I2C
#include <Arduino.h>
#include "USM_Input.h"                                                      // For input handling (embedded)
#include "LiquidCrystal_I2C.h"                                              // #include <LiquidCrystal_I2C.h>
#include "Adafruit_MCP23017.h"                                              // For mcp23017 I/O buffers

// Global Variables ----------------------------------------------------------------------------------------------------------------------------------

uint32_t watchdogLastResetMS_Gm = 0L;                                       // Last time the watchdog was reset
uint8_t  binaryValue_Gm[4] = {0,0,0,0};
uint8_t  inMCPsFound_Gm = 0;                                                // Each bit corresponds to an mcp found on the IC2 bus

char message[200]; // 165
char switchNumber_Gm[3];
char switchRelay_Gm[3];
char switchStyle_Gm[8];
char switchType_Gm[3];
char switchLocation_Gm[15];
char switchUsage_Gm[7];
char switchEvent_Gm[7];

boolean currentState_Gm[96];
boolean showConsole_Gm = true;
boolean showDisplay_Gm = true;

// Instantiate Global Objects ------------------------------------------------------------------------------------------------------------------------

LiquidCrystal_I2C LCD_ADDRESS(0x27, 20, 4, LCD_5x10DOTS);
Adafruit_MCP23017 IN_MCP23017[8];                                           // I/O out buffers
Adafruit_MCP23017 OUT_MCP23017[8];                     
USM_Input USM_INPUT[8];                                                     // Input handlers
void usmEvent(uint8_t mcp, uint8_t input, uint8_t type, uint8_t state);

// Program -------------------------------------------------------------------------------------------------------------------------------------------

void initialiseWatchdog()
{
  if (ENABLE_WATCHDOG) {
    Serial.print(F("Watchdog ENABLED on pin "));
    Serial.println(WATCHDOG_PIN);
    pinMode(WATCHDOG_PIN, OUTPUT);
    digitalWrite(WATCHDOG_PIN, LOW);
  }
  else {
    Serial.println(F("Watchdog NOT enabled")); 
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void patWatchdog()
{
  if (ENABLE_WATCHDOG) {
    if ((millis() - watchdogLastResetMS_Gm) > WATCHDOG_RESET_MS) {
      digitalWrite(WATCHDOG_PIN, HIGH);
      delay(WATCHDOG_PULSE_MS);
      digitalWrite(WATCHDOG_PIN, LOW);
      watchdogLastResetMS_Gm = millis();
    }
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void selectRelay(uint8_t index0)
{ 
  currentState_Gm[index0] = !currentState_Gm[index0];

  uint8_t relayIndex = SWITCH_RELAY_Gc[index0];
  uint8_t relayBoardNumber = ceil(((int)relayIndex - 1) / 8);
  uint8_t relayPinPosition = relayIndex - (relayBoardNumber * 8);
  
  if (currentState_Gm[index0]) {
    byte mask = 1 << (relayPinPosition - 1);
    binaryValue_Gm[relayBoardNumber] = binaryValue_Gm[relayBoardNumber] | mask;
  }
  else {
    byte mask = 255 - ( 1 << (relayPinPosition - 1));
    binaryValue_Gm[relayBoardNumber] = binaryValue_Gm[relayBoardNumber] & mask;
  }
  
  Wire.beginTransmission(OUT_MCP_ADDRESS_Gc[relayBoardNumber]);
  Wire.write(0x12);
  Wire.write(binaryValue_Gm[relayBoardNumber]);
  Wire.endTransmission();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
 
void getSwitchEvent(uint8_t type, uint8_t state) // char switchEvent_Gm[7];
{
                             sprintf_P(switchEvent_Gm, PSTR("Error "));
  switch (type) {
    case BUTTON: switch (state) {
        case USM_HOLD_EVENT: sprintf_P(switchEvent_Gm, PSTR("Hold  ")); break;
        case 1:              sprintf_P(switchEvent_Gm, PSTR("Single")); break;
        case 2:              sprintf_P(switchEvent_Gm, PSTR("Double")); break;
        case 3:              sprintf_P(switchEvent_Gm, PSTR("Triple")); break;
        case 4:              sprintf_P(switchEvent_Gm, PSTR("Quad  ")); break;
        case 5:              sprintf_P(switchEvent_Gm, PSTR("Penta ")); break;
      }                                                                 break;
    case CONTACT: switch (state) {
        case USM_LOW:        sprintf_P(switchEvent_Gm, PSTR("Closed")); break;
        case USM_HIGH:       sprintf_P(switchEvent_Gm, PSTR("Open  ")); break;
      }                                                                 break;
    case ROTARY:
      switch (state) {
        case USM_LOW:        sprintf_P(switchEvent_Gm, PSTR("Up    ")); break;
        case USM_HIGH:       sprintf_P(switchEvent_Gm, PSTR("Down  ")); break;
      }                                                                 break;
    case SWITCH:
      switch (state) {
        case USM_LOW:        sprintf_P(switchEvent_Gm, PSTR("On    ")); break;
        case USM_HIGH:       sprintf_P(switchEvent_Gm, PSTR("Off   ")); break;
      }                                                                 break;
    case TOGGLE:             sprintf_P(switchEvent_Gm, PSTR("Toggle")); break;
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void getSwitchUsage(uint8_t use) // char switchUsage_Gm[7];
{
             sprintf_P(switchUsage_Gm, PSTR("Error ")); 
  switch (use) {
    case 0:  sprintf_P(switchUsage_Gm, PSTR("Light ")); break;
    case 1:  sprintf_P(switchUsage_Gm, PSTR("Light2")); break;
    case 2:  sprintf_P(switchUsage_Gm, PSTR("Fan   ")); break;
    case 3:  sprintf_P(switchUsage_Gm, PSTR("Fan 2 ")); break;
    case 4:  sprintf_P(switchUsage_Gm, PSTR("Window")); break;
    case 5:  sprintf_P(switchUsage_Gm, PSTR("Door  ")); break;
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void getSwitchLocation(uint8_t room) // char switchLocation_Gm[15];
{
             sprintf_P(switchLocation_Gm, PSTR("Error         ")); 
  switch (room) {
    case 0:  sprintf_P(switchLocation_Gm, PSTR("Rear Patio    ")); break;
    case 1:  sprintf_P(switchLocation_Gm, PSTR("Main Bathroom ")); break;
    case 2:  sprintf_P(switchLocation_Gm, PSTR("Ensuite Toilet")); break;
    case 3:  sprintf_P(switchLocation_Gm, PSTR("Ensuite Bathrm")); break;
    case 4:  sprintf_P(switchLocation_Gm, PSTR("Ensuite Bathrm")); break;
    case 5:  sprintf_P(switchLocation_Gm, PSTR("Kitchen       ")); break;
    case 6:  sprintf_P(switchLocation_Gm, PSTR("Laundry       ")); break;
    case 7:  sprintf_P(switchLocation_Gm, PSTR("Living Room   ")); break;
    case 8:  sprintf_P(switchLocation_Gm, PSTR("Dining Room   ")); break;
    case 9:  sprintf_P(switchLocation_Gm, PSTR("Bedroom 1     ")); break;
    case 10: sprintf_P(switchLocation_Gm, PSTR("Bedroom 2     ")); break;
    case 11: sprintf_P(switchLocation_Gm, PSTR("Bedroom 3     ")); break;
    case 12: sprintf_P(switchLocation_Gm, PSTR("Bedroom 4     ")); break;
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void getSwitchType(uint8_t type) // char switchType_Gm[3];
{
                  sprintf_P(switchType_Gm, PSTR("Er")); 
  switch (type) {
    case BUTTON:  sprintf_P(switchType_Gm, PSTR("DA")); break;
    case CONTACT: sprintf_P(switchType_Gm, PSTR("SA")); break;
    case ROTARY:  sprintf_P(switchType_Gm, PSTR("SA")); break;
    case SWITCH:  sprintf_P(switchType_Gm, PSTR("SA")); break;
    case TOGGLE:  sprintf_P(switchType_Gm, PSTR("SA")); break;
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void getSwitchStyle(uint8_t type) // char switchStyle_Gm[8];
{
                  sprintf_P(switchStyle_Gm, PSTR("Error  ")); 
  switch (type) {
    case BUTTON:  sprintf_P(switchStyle_Gm, PSTR("Button ")); break;
    case CONTACT: sprintf_P(switchStyle_Gm, PSTR("Contact")); break;
    case ROTARY:  sprintf_P(switchStyle_Gm, PSTR("Rotary ")); break;
    case SWITCH:  sprintf_P(switchStyle_Gm, PSTR("Switch ")); break;
    case TOGGLE:  sprintf_P(switchStyle_Gm, PSTR("Toggle ")); break;
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void getSwitchRelay(uint8_t relay) // char switchRelay_Gm[3];
{
  String msg_str = String(relay); char x[3]; 
  msg_str.toCharArray(x, msg_str.length() + 1);
  sprintf_P(switchRelay_Gm, PSTR(x)); 
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void getSwitchNumber(uint8_t index1) // char switchNumber_Gm[3];
{
  String msg_str = String(index1); char x[3]; 
  msg_str.toCharArray(x, msg_str.length() + 1);
  sprintf_P(switchNumber_Gm, PSTR(x)); 
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void showData(uint8_t index0, uint8_t type, uint8_t state)
{
  getSwitchNumber(index0 +1);                                               // switchNumber_Gm[3]    - Switch Number
  getSwitchRelay(SWITCH_RELAY_Gc[index0]);                                  // switchRelay_Gm[3]     - Relay Number
  getSwitchStyle(type);                                                     // switchStyle_Gm[8]     - Switch Style
  getSwitchType(type);                                                      // switchType_Gm[3]      - Switch Type
  getSwitchLocation(SWITCH_LOCATION_Gc[index0]);                            // switchLocation_Gm[15] - Switch Location
  getSwitchUsage(SWITCH_USAGE_Gc[index0]);                                  // switchUsage_Gm[8]     - Switch Usage
  getSwitchEvent(type, state);                                              // switchEvent_Gm[7]     - Switch Event

  if (showConsole_Gm) {
    sprintf(message, "{ Switch Number = %s, Relay Number = %s, Switch Style = %s, Switch Type = %s, Switch Location = %s, Switch Usage = %s, Switch State = %s }",
    switchNumber_Gm, switchRelay_Gm, switchStyle_Gm, switchType_Gm, switchLocation_Gm, switchUsage_Gm, switchEvent_Gm);
    Serial.println(message);
  }

  if (Serial1) {
    sprintf(message, "{\"Switch Number\":%s,\"Relay Number\":%s,\"Switch Style\": \"%s\",\"Switch Type\": \"%s\",\"Switch Location\": \"%s\",\"Switch Usage\": \"%s\",\"Switch Event\": \"%s\"}",
    switchNumber_Gm, switchRelay_Gm, switchStyle_Gm, switchType_Gm, switchLocation_Gm, switchUsage_Gm, switchEvent_Gm);
    Serial1.println(message);
  }

  if (showDisplay_Gm) {
    LCD_ADDRESS.backlight();
    // Line 1 - Switch Number: & Relay Number:  
    LCD_ADDRESS.setCursor(0,0);  LCD_ADDRESS.printf("Switch:%s", switchNumber_Gm);
    LCD_ADDRESS.setCursor(12,0); LCD_ADDRESS.printf("Relay:%s",  switchRelay_Gm);
    // Line 2 - Style: & Type:
    LCD_ADDRESS.setCursor(0,1);  LCD_ADDRESS.printf("Styl:%s",   switchStyle_Gm);
    LCD_ADDRESS.setCursor(13,1); LCD_ADDRESS.printf("Type:%s",   switchType_Gm);
    // Line 3 - Location:
    LCD_ADDRESS.setCursor(0,2);  LCD_ADDRESS.printf("Loc:%s",    switchLocation_Gm);
    // Line 4 - Usage:
    LCD_ADDRESS.setCursor(0,3);  LCD_ADDRESS.printf("Use:%s",    switchUsage_Gm);
    LCD_ADDRESS.setCursor(10,3); LCD_ADDRESS.printf("Cur:%s",    switchEvent_Gm);
  }
  else {
    LCD_ADDRESS.noBacklight();
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void usmEvent(uint8_t mcp, uint8_t pin, uint8_t type, uint8_t state)
{
  uint8_t index0 = (MCP_PIN_COUNT * mcp) + pin;

  switch (type) {
    case BUTTON: switch (state) {
        case USM_HOLD_EVENT:         break;
        case 1: selectRelay(index0); break;
        case 2:                      break;
        case 3:                      break;
        case 4:                      break;
        case 5:                      break;
      }                              break;
    case CONTACT: switch (state) {
        case USM_LOW:  if (currentState_Gm[index0])  { selectRelay(index0); }; break;
        case USM_HIGH: if (!currentState_Gm[index0]) { selectRelay(index0); }; break;
      }                                                                        break;
    case ROTARY:
      switch (state) {
        case USM_LOW:  break;
        case USM_HIGH: break;
      }                break;
    case SWITCH:
      switch (state) {
        case USM_LOW:  break;
        case USM_HIGH: break;
      }                break;
    case TOGGLE:       break;
  }
  showData(index0, type, state);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void incomingButtonPress()
{
  if (Serial1.available()) {
    uint8_t index = Serial1.parseInt(); //-1;
    Serial.print("Inbound data from a source other than LSC i.e. Node Red, openHAB, Terminal window = "); Serial.println(index);
    if (index > 0 && index < 97) {                                          // Raw button number
      selectRelay(index -1);                                                // Convert to zero index
    }
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void loop()
{ 
  incomingButtonPress();

  for (uint8_t mcp = 0; mcp < 8; mcp++) {                                   // Iterate through each of the mcp23017 input buffers
    if (bitRead(inMCPsFound_Gm, mcp) == 0) continue;
    uint16_t io_value = IN_MCP23017[mcp].readGPIOAB();                      // Read the values for all 16 inputs on this mcp
    USM_INPUT[mcp].process(mcp, io_value);                                  // Check for any input events
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(BAUDRATE, SERIAL_8N1, RXS0, TXS0);
  Serial1.begin(BAUDRATE, SERIAL_8N1, RXS1, TXS1);
  Wire.begin(SDA_1, SCL_1, FREQUENCY);
  Wire1.begin(SDA_2, SCL_2, FREQUENCY);
  pinMode(IN_PIN, INPUT_PULLUP);
  
  uint8_t outMCPsFound = 0;
  uint8_t valueIn[8];
  uint8_t valueOut[8];
  uint8_t index0 = 0;
  // ------------------------ In MCPs ------------------------
  for (uint8_t mcp = 0; mcp < 8; mcp++) { 
    Wire1.beginTransmission(IN_MCP_ADDRESS_Gc[mcp]); 
    bitWrite(inMCPsFound_Gm, mcp, Wire1.endTransmission() == 0);    
    
    if (bitRead(inMCPsFound_Gm, mcp)) { 
      IN_MCP23017[mcp].begin(mcp, &Wire1);
      
      for (uint8_t pin = 0; pin < 16; pin++) {
        IN_MCP23017[mcp].pinMode(pin, INPUT);
        IN_MCP23017[mcp].pullUp(pin, HIGH);       
        // Turning on the relays directly and bypassing any button presses that might have undesirable results at start up,
        // upon boot if the switch is already on, then turn on the corresponding relay (if relay assigned), otherwise keep them off.
        if      ( SWITCH_TYPE_Gc[index0] == 1 && 1 == IN_MCP23017[mcp].digitalRead(pin) ) { selectRelay(index0); }
        else if ( SWITCH_TYPE_Gc[index0] == 0 && 0 == IN_MCP23017[mcp].digitalRead(pin) ) { selectRelay(index0); }

        switch (SWITCH_TYPE_Gc[index0]) {
          case 0: USM_INPUT[mcp].setType(pin, BUTTON);  break;
          case 1: USM_INPUT[mcp].setType(pin, CONTACT); break;
          case 2: USM_INPUT[mcp].setType(pin, ROTARY);  break;
          case 3: USM_INPUT[mcp].setType(pin, SWITCH);  break;
          case 4: USM_INPUT[mcp].setType(pin, TOGGLE);  break;
        }
        index0 ++;
      }
      USM_INPUT[mcp].onEvent(usmEvent); // Needed to display button presses during / after startup.
      valueIn[mcp] = 255; } else { valueIn[mcp] = 88;
    }
  }
  // ------------------------ Out mcps ------------------------
  for (uint8_t mcp = 0; mcp < 8; mcp++) {
    Wire.beginTransmission(OUT_MCP_ADDRESS_Gc[mcp]);
    bitWrite(outMCPsFound, mcp, Wire.endTransmission() == 0);

    if (bitRead(outMCPsFound, mcp)) {
      OUT_MCP23017[mcp].begin(mcp, &Wire);

      for (uint8_t pin = 0; pin < 8; pin++) {
        OUT_MCP23017[mcp].pinMode(pin, OUTPUT);
        OUT_MCP23017[mcp].pullUp(pin, HIGH);
      }
      valueOut[mcp] = 255; } else { valueOut[mcp] = 88;
    }
  }
  // ------------------------ Display ------------------------
  if (valueOut[7] == 255) {
    Wire.beginTransmission(0x27);
    bitWrite(outMCPsFound, 7, Wire.endTransmission() == 0);

    if (bitRead(outMCPsFound, 7)) {      
      LCD_ADDRESS.begin(); LCD_ADDRESS.backlight();
      LCD_ADDRESS.setCursor(0,0); LCD_ADDRESS.print("<-----Coded-By----->");
      LCD_ADDRESS.setCursor(0,1); LCD_ADDRESS.print("|     D.B.E        |");
      LCD_ADDRESS.setCursor(0,2); LCD_ADDRESS.print("|     v2.00        |");
      LCD_ADDRESS.setCursor(0,3); LCD_ADDRESS.print("<-----May-2021----->");
      
      delay(500); LCD_ADDRESS.clear(); delay(500);
      LCD_ADDRESS.setCursor(0,0); LCD_ADDRESS.print("  I2C Address 0x    ");
      LCD_ADDRESS.setCursor(0,2); LCD_ADDRESS.print("    IN:             ");
      LCD_ADDRESS.setCursor(0,3); LCD_ADDRESS.print("   OUT:             ");

      Serial.println(); Serial.println("      IN: ");
      for (uint8_t mcp = 0; mcp < 8; mcp++) {
        LCD_ADDRESS.setCursor(16,0); LCD_ADDRESS.print(IN_MCP_ADDRESS_Gc[mcp], HEX);
        LCD_ADDRESS.setCursor(8 + mcp, 2); LCD_ADDRESS.write(valueIn[mcp]);        
        Serial.print("0x"); Serial.print(IN_MCP_ADDRESS_Gc[mcp], HEX);
        if (valueIn[mcp] == 255) { Serial.println(" = Present"); } else { Serial.println(" = Not Present"); }
        delay(100);
      }

      Serial.println(); Serial.println("     OUT: ");
      for (uint8_t mcp = 0; mcp < 8; mcp++) {
        LCD_ADDRESS.setCursor(16,0); LCD_ADDRESS.print(OUT_MCP_ADDRESS_Gc[mcp], HEX);
        LCD_ADDRESS.setCursor(8 + mcp, 3); LCD_ADDRESS.write(valueOut[mcp]);        
        Serial.print("0x"); Serial.print(OUT_MCP_ADDRESS_Gc[mcp], HEX);
        if (valueOut[mcp] == 255) { Serial.println(" = Present"); } else { Serial.println(" = Not Present"); }
        delay(100);
      }
      delay(500); LCD_ADDRESS.noBacklight(); LCD_ADDRESS.clear();
    }
  }

  Serial.println(); initialiseWatchdog(); 
  Serial.println("Finished Setup()"); Serial.println();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
