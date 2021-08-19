#include <Arduino.h>

/* Configuration File... This is where the magic happens...
Screen accomodates 20 x 4 characters. Exceeding this will cause the screen to wrap around and not perform as expected
0                  19
xxxxxxxxxxxxxxxxxxxx 0
xxxxxxxxxxxxxxxxxxxx 1
xxxxxxxxxxxxxxxxxxxx 2
xxxxxxxxxxxxxxxxxxxx 3

Each letter, number or space uses up one space on the screen.       0123456789..........
The first line displays the Switch Number and the Relay Number:     Switch:nn   Relay:nn

The Switch Numbers below are fixed, and are determined by their position on the Light Switch Controller (LSC)

[1- 4] [9- 12] [17-20] [25-28] [33-36] [41-44] [49-52] [57-60] [65-68] [73-76] [81-84] [89-92]
[5- 8] [13-16] [21-24] [29-32] [37-40] [45-48] [53-56] [61-64] [69-72] [77-80] [85-88] [93-96] */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------

// Relay numbering starts from 1. Don't use a number higher than the amount of relays you have i.e max 32 or it will simply not work.
// You can have multiple buttons or sensors trigger the same relay by putting a given relay number in that button spot. i.e. 5, 5, 7, 5, 8, 9 for multi-way light switches 

const uint8_t SWITCH_RELAY_Gc[96] = {
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

// ------------------------------------------------------------------------------------------------------------------------------------------------------------

// Modify as required... Button = 0, Contact = 1, Rotary = 2, Switch = 3, Toggle = 4, currently only 0 and 1 will work, as the other have no code associated with them

const uint8_t SWITCH_TYPE_Gc[96] = { 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// ------------------------------------------------------------------------------------------------------------------------------------------------------------

// Modify as required... See main code "void getSwitchLocation(uint8_t room)" to see what the numbers relate to, or if you need to make room changes

const uint8_t SWITCH_LOCATION_Gc[96] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7,
8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3,
4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

// ------------------------------------------------------------------------------------------------------------------------------------------------------------

// Modify as required... See main code "void getSwitchUsage(uint8_t use)" to see what the numbers relate to, or if you need to make usage changes

const uint8_t SWITCH_USAGE_Gc[96] = {
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3,
2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 };

// ------------------------------------------------------------------------------------------------------------------------------------------------------------

const byte IN_MCP_ADDRESS_Gc[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
const byte OUT_MCP_ADDRESS_Gc[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };

// ------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef CONFIG_H
#define CONFIG_H

/* ----------------- General config -------------------------------- */
/* Debugging */
#define ENABLE_DEBUG                  true

/* Serial */
#define SERIAL_BAUD_RATE              9600 // 115200

/* ----------------- Hardware-specific config ---------------------- */
/* Watchdog */
#define ENABLE_WATCHDOG         false
#define WATCHDOG_PIN            3                           // Output to pat the watchdog
#define WATCHDOG_PULSE_MS       50
#define WATCHDOG_RESET_MS       30000

// These settings work with my setup, they may not work with yours, alter as required.

#define RXS0 4                                                  
#define TXS0 5                                              // dev/ttyUSB0 (laptop via the TP USB-C port)
#define RXS1 18                                             // dev/ttyS0 over on RPi via RX-TX pins
#define TXS1 19                                             // dev/ttyS0 over on RPi via RX-TX pins
#define SDA_1 21                                            // SDA on RSC
#define SCL_1 22                                            // SCL on RSC
#define SDA_2 33                                            // SDA on LSC
#define SCL_2 32                                            // SCL on LSC
#define IN_PIN 23                                           // Pin for LCD screen activation
#define BAUDRATE 9600
#define FREQUENCY 100000                                    // For setting Wire and Wire1 Frequency
#define MCP_PIN_COUNT 16                                    // Each MCP23017 has 16 inputs

#endif

// ------------------------------------------------------------------------------------------------------------------------------------------------------------
