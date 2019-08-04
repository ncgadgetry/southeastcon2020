# Southeastcon 2020 Arena control code

This is the Arduino control code for the arena. It is designed to run
on an Arduino Mega (1280 or 2560).

The 10 push buttons and 10 LEDs are wired to pins 26 - 35 as shown below:

     ------  -- -- -- -- -- -- -- -- -- --
     ID #     0  1  2  3  4  5  6  7  8  9
     ------  -- -- -- -- -- -- -- -- -- --
     LED     26 28 30 32 34 36 38 40 42 44 
     BUTTON  27 29 31 33 35 37 39 41 43 45
     ------  -- -- -- -- -- -- -- -- -- --

The LEDs are wired such that the cathodes of all the LEDs are tied to
ground and the anodes tied to the pins listed above.

The button switches are wired such that one half of each button is
connected to ground and the other side to the pin listed above.

Note it is possible to run a single ground wire across all 10x buttons
tying the 20x points of the LEDs and switches together. To make it 
easier, I used a 22 gauge uninsulated wire and just stiched it through
all the pins (orienting the buttons so that the LED cathonde and one
side of the switch is nearest the floor.

That leaves one ground connection for all 10x switches, followed by
20x additional wires as shown in the table above.

This should be clear enough without a schematic, but if there is any
question, please let me know.

