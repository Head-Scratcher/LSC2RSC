#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_MCP23017.h>
#define DEBOUNCE_DURATION 25
const int ledPin = 13;
Adafruit_MCP23017 inMCPs[6];
boolean FTT[96] = {true};

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
        { FTT[inNum] = false;
          timeStart = millis();
        }
        if (millis()-timeStart > DEBOUNCE_DURATION)
        {
          timeStart = millis();
          digitalWrite(ledPin, HIGH);
          while (1 != inMCPs[i].digitalRead(j)) {}
        }
      }
    else
    { 
      if (!FTT[inNum])
      {
        digitalWrite(ledPin, LOW);
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
  Wire.begin();
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
