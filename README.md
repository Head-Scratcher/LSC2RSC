# LSC2RSC
Light Switch Controller 2 Relay Shield Carrier

This code was written by Paul Murray to be used as "base code" to allow a SuperHouse 24 Channel I2C RJ45 Breakout Board (SKU: i2crj45x24) by Jon Oxer
to be used with a SuperHouse Relay Shield Carrier x4 (SKU: RSC - https://www.superhouse.tv/product/relay-shield-carrier-x4/) 
and four SuperHouse 8-Channel Relay Driver Shield (SKU: RELAY8 - https://www.superhouse.tv/product/8-channel-relay-driver-shield/)

The board used is an ESP32 based TinyPICO board by Seon Rozenblum (https://www.tinypico.com/ - https://unexpectedmaker.com/)

The code may work with other boards, but it hasn't been tested on other boards. I would however make mention that it utilises two I2C buses, so the board would need to support two or more I2C buses.

This code is the result of a calborative effort. While I used very little of the original example code I was provided, it was that example code that provided the insights to finally come up what you see here.

I would also like to thank some of the people on the SuperHouseTV Discord Server, without their code help, comments, encouragement, and general direction, this code may never of happened. In no particular order I would like to thank JamesKennewell, aitken85, Austins_Creations, Jon Oxer, Tinker, sarreck, and sumnerboy. I really hope I didn't forget anyone, and if I did I'm sorry.
