/**
 * SoutheastCon 2029 Arena control - ArenaControl.cpp
 *
 * Pin assignments, as well as a challenge to the code reviwers
 *    to find bugs, and the list of those who helped, are listed
 *    in the ArenaControl.h file
 *
 */
 
#include "Arduino.h"
#include "ArenaControl.h"
#include "pi2000.h"


/**
 * readSwitch - returns true if switch is depressed, else false 
 */
boolean buttonPressed(int id) {
   return !digitalRead(BUTTON_PIN(id));   
}


/**
 * setLED - turns on the LED is is_on is true, else turns it off
 */
void setLED(int id, boolean is_on) {
   digitalWrite(LED_PIN(id), (is_on ? HIGH : LOW));
}


/**
 * setAllLEDs - sets all the LEDs to either on or off
 */
void setAllLEDs(boolean is_on) {

   int led_id;
   
   for (led_id=0; led_id < NUM_BUTTONS; led_id++) {
      setLED(led_id, is_on);
   }
}


/**
 * flashAllLEDs - turns all the LEDs on, and sets a timeout of when to
 *    turn them all off. If called again before the timeout is up, it 
 *    just resets the timeout (so a fast robot continuing to press the
 *    wrong button will not see the LEDs go off, but will continue to 
 *    count as invalid digits if the delay is more than the timeout)
 */
void flashAllLEDs() {

   // Turn on all the LEDs, and set a time to turn them off again
   setAllLEDs(true);
   flashTimeout = millis() + FLASH_INTERVAL;
}


/**
 * Scan all the buttons, debouncing each, and sets newPress true if at 
 *   least one of the buttons transitioned from not pressed to pressed
 *   on this pass, and returns the count of on buttons in numPressed
 * 
 * The debounce logic is based on the example here of one button:
 *    http://www.arduino.cc/en/Tutorial/Debounce
 */
void debounceButtons(boolean *newPress, int *numPressed) {

   int  digit;
   
   *newPress = false;
   *numPressed = 0;
   
   // Scan each of the buttons   
   for (digit=0; digit < NUM_BUTTONS; digit++) {
      
     // read the state of the switch into a local variable:
     int reading = buttonPressed(digit);
   
     // check to see if you just pressed the button
     // (i.e. the input went from false to true), and you've waited long enough
     // since the last press to ignore any noise:   
     
     // If the switch changed, due to noise or pressing:
     if (reading != buttonState[digit].lastButtonState) {
        
       // reset the debouncing timer
       buttonState[digit].lastDebounceTime = millis();
     }
   
     // whatever the reading is at, it's been there for longer than the debounce
     // delay, so take it as the actual current state:
     if ((millis() - buttonState[digit].lastDebounceTime) > DEBOUNCE_DELAY) {
  
       // if the button state has changed:
       if (reading != buttonState[digit].buttonState) {
         buttonState[digit].buttonState = reading;

         // If any button was just pressed, we set the new press boolean
         if (buttonState[digit].buttonState) {
            *newPress = true;
         }
       }
     }

   // If this button is on, increment our count of numPressed       
   if (buttonState[digit].buttonState) {
      (*numPressed)++;
   }
   
   // save the reading. Next time through the loop, it'll be the lastButtonState:
   buttonState[digit].lastButtonState = reading;
   }
}


/**
 * Do whatever set up is required to wait for the competition to start,
 *    and to display the first digit
 */
void startCompetition() {

   boolean newPress;
   int numPressed;
   
   // The judge starts the competition by pressing any one button
   do {
      debounceButtons(&newPress, &numPressed);      
   } while (!newPress);

   // But delay here until the judge releases the button
   do {
      debounceButtons(&newPress, &numPressed);      
   } while (numPressed == 0);

   // Quick flash to indicate the competition has begun
   setAllLEDs(true);
   delay(FLASH_INTERVAL);
   setAllLEDs(false);
   
   // Now light up the first digit ('3') to start the competition
   setLED(3, true);
}


/**
 * endCompetition - turn on all LEDs, show score, halt
 */
void endCompetition() {
      
   // Turn on all the LEDs to indicate the competition is over
   setAllLEDs(true);

   // If no sequence errors so far, num sequenced is our position!   
   if (inSequence) {
      numSequenced = piDigitPosn;
   }
         
   // Print out the stats on presses
   Serial.println(F("Time's up\n"));
   Serial.print(F("Num sequenced currently:"));
   Serial.println(numSequenced);
   Serial.print(F("Extra not sequenced currently:"));
   Serial.println(extraNotSequenced);
   
   // Calculate and print out the score - 10 points for each one
   //   sequenced correctly, plus 1 point (max of 100) for those not
   //   sequenced correctly
   int score = (numSequenced * 10) + min(extraNotSequenced, 100);
   Serial.print(F("\nFinal score: "));
   Serial.println(score);
   
   // Wait here forever...
   for(;;);
}


/**
 * setup - this is executed once at the start of the sketch, so perform any
 *    necessary setup operations here
 */
void setup() {

   int i;
   
   Serial.begin(9600);

   // Setup our buttons as input and our leds as output, and turn the LEDs off
   for (i=0; i < NUM_BUTTONS; i++) {
      pinMode(LED_PIN(i), OUTPUT);
      pinMode(BUTTON_PIN(i), INPUT_PULLUP);
      setLED(i, false);
   }
   
   // Wait for the competition to start
   startCompetition();
   
   // Save off the timestamp of when we started the competition
   startTimestamp = millis();
}


/**
 * loop - called continuously for as long as the Arduino is on, so perform
 *   any operations here. All operations are performed as part of a state 
 *   machine so no delays in the loop processing
 */
void loop() {

   boolean newPress;
   int numPressed;
   
   // If the match is over, stop and report the score now and halt here
   if ((millis()-startTimestamp) >= MATCH_RUNTIME) {
      endCompetition();  // Will not return from this
   }

   // If we are 'flashing' all LEDs, see if the flash on interval is over
   if ((flashTimeout != 0) && (millis() > flashTimeout)) {
      setAllLEDs(false);
   }
   
   // Debounce all the buttons...   
   debounceButtons(&newPress, &numPressed);
   
   // If no buttons are pressed, then nothing else for us to do
   if (numPressed == 0) {
   }

   // Else we handle the case of currently in a sequence   
   else if (inSequence) {

      // If more than one button is pressed, we are no longer sequencing correctly
      if (numPressed > 1) {
         inSequence = false;
         numSequenced = piDigitPosn;
         flashAllLEDs();
      }

     // Else exactly one was pressed, and this is a new press
     else if (newPress) {
        
        // Get the 0..9 value of the current sequence digit
        int digit = pi[piDigitPosn] - '0';

        // If the wrong button was pressed, we are no longer sequencing
        if (buttonState[digit].buttonState == false) {
           inSequence = false;
           numSequenced = piDigitPosn;
           flashAllLEDs();

        // Else new press, correct so turn off digit, increment count of correct
        //   digits and light up the next digit
        } else {
           setLED(digit, false);
           piDigitPosn++;
           digit = pi[piDigitPosn] - '0';
           setLED(digit, true);
        }
     }
   }
   
   // And finally we handle the case of not in a sequence
   else {

      // If this is a new press, count it and flash all the digits
      if (newPress) {
         extraNotSequenced++;
         flashAllLEDs();
      }
   }
}


