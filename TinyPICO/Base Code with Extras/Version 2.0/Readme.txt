After months of trying to get this software working, I am now in a place where I am happy to release it. Version 2.0 represents a significant change in thinking compared to the 1.x versions.

While they all worked fine, one of the biggest draw backs to them was sending a button press off to the Raspberry Pi, for it to be matched with a corresponding relay number and be sent back. Due to the lengthy / random time taken for it to process the request, I decided to move away from external matching.

It always had a fall back matrix locally where it could retrieve a relay number from in the event a response from the Raspberry Pi took too long (> 1 second), or didn't respond at all, but I needed to have some kind of acceptable response time and consistency.

It is my hope that the software is now more robust by matching the relay numbers locally only. I did however still want to be able to receive button presses from external sources, and as such, the software on the TinyPICO is capable of receiving messages over Serial when the software on a Raspberry Pi is running. Likewise the software is also capable of sending messages in an outbound direction to things like MQTT, Node Red and openHAB via the Raspberry Pi.

One of the things I wanted to change over version 1.x was the reliance on the Raspberry Pi to fulfil its purpose successfully. The new software will only send / receive messages to / from a Raspberry Pi if it senses its presence, or you tell it to, otherwise it will just carry on without it. The same is true for an external screen. While I recommend a Raspberry Pi and screen to get the maximum benefit from the software, I didn't want the software to be dependant on it.

The other major change in Version 2.0 is the software now leverages the USM_Input library whereas the previous versions used the LSC_Input library. What this does is opens up more ways to trigger the relays through different switch types. At the moment the software only takes input from Normally Open button switches, and Normally Closed reed switches like those used on windows and doors. I have left room in the code for expansion to other switch types should there be a need in the future.

There are several major contributors to this software that I need to recognise. Some of these people offered help that may have been used at some point and subsequently removed, but they all contributed to the journey, and I want to recognise them. I have no intentions of trying to remember all of them at the time of writing this Readme.txt, but I will add them as they identify themselves or I remember.

So in no particular order I will start with sumnerboy and moin, whose biggest contribution comes in the form of the USM_Input library. I didn't really have time during its development to follow closely what they were doing, but I believe their work was based largely on someone else work, who I am fairly sure that they mention who in their files, but their contribution made some significant and major changes to it, and you now have different switch types thanks to them. 

Ordinarily I trim comments at the start of the file to get to the meat of the code sooner (it's an OCD thing), but I will add their files USM_Input.cpp and USM_Input.h exactly as they wrote them, so they can credit who they need to, and make sure I don't remove anything of importance.

I want to get these files uploaded, so watch this space for more names to appear, there are more people to credit.

Ohh and of course I accept no responsibility for the software, provided as is, education use only etc, etc, you know the drill.

Regards,
Paul

EDIT: OK so looking through Discord conversations it was Lusu who provided me with a cut down alternative to the LSC_Input file. I went back and forth several times before ultimately deciding to use the USM_Input library, so while it didn't appear in the final code, I am extremely grateful for all the help Lusu provided.

Just looking at who else I have had convesation with... Varying degrees of help, but I am sure all of the following have provided at least some help.
Chris Aitken, Tinker, Austins_Creations, JamesKennewell, FrankMc, UnexpectedMaker, JonOxer.

Still can't help but think I am missing a few...
