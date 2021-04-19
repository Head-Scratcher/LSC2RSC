// This code tests just the i2crj45x24 button presses. It will illuminate the TinyPICO onboard LED while the button is pressed, and go off when released.

#include <Wire.h>
#include <Arduino.h>
#include <TinyPICO.h>
#include <Adafruit_MCP23017.h>
#define DEBOUNCE_DURATION 25
Adafruit_MCP23017 inMCPs[6];
TinyPICO tp = TinyPICO();
boolean FTT[96] = {true};
const int ledPin = 13;

/* ========================================================================= */

void loop()
{

  int inNum = 1;
  unsigned long timeStart = 0;
  for (unsigned char i = 0; i < 6; i++)
  {
    for (unsigned char j = 0; j < 16; j++)
    {
      if (1 != inMCPs[i].digitalRead(j))
      { 
        if (FTT[inNum])
        {
          FTT[inNum] = false;
          timeStart = millis();
        }
        if (millis()-timeStart > DEBOUNCE_DURATION)
        {
          timeStart = millis();
          tp.DotStar_SetPixelColor( 255, 128, 0 );
          while (1 != inMCPs[i].digitalRead(j))
          {
          }
        }
      }
    else
    {
      if (!FTT[inNum])
      {
        tp.DotStar_SetPixelColor( 0, 0, 0 );
        FTT[inNum] = true;
      }
    }
      inNum++;
    }
  }

}

/* ========================================================================= */

void setup()
{

  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  Wire.begin( 21, 22, 100000);
  for (unsigned char j = 0; j < 6; j++)
  {
    inMCPs[j].begin(j, &Wire);
    for (unsigned char k = 0; k < 16; k++)
    {

      inMCPs[j].pinMode(k, INPUT);
      inMCPs[j].pullUp(k, HIGH);
    }
  }
 
}

/* ========================================================================= */
