/**
 * SoutheastCon 2020 Arena control - ArenaControl.cpp
 *
 * Pin assignments, as well as a challenge to the code reviwers
 *    to find bugs, and the list of those who helped, are listed
 *    in the ArenaControl.h file
 *
 */
 
#include "Arduino.h"
#include "ArenaControl.h"
#include "pi10000.h"

// Libraries for the optional LCD controller
#include <Wire.h>
#include "Sainsmart_I2CLCD.h"

// Defines and globals for the optional LCD controller
#define LCD_ADDRESS   0x27
#define NUM_COLUMNS   20
#define NUM_ROWS      4
#define BUTTON_START  A0
#define BUTTON_STOP   A3
Sainsmart_I2CLCD lcd(LCD_ADDRESS, NUM_COLUMNS, NUM_ROWS);
boolean hasController = false;


/**
 * buttonPressed - returns true if switch is depressed, else false 
 *   
 * The switch input is active-low, as the other side of the switch
 *    is tied to ground. This allows connecting all 10x of the 
 *    switches to all 10x of the LED returns together with one
 *    ground wire (see photos in hw-uploads channel)
 */
boolean buttonPressed(int id) {
   return !digitalRead(BUTTON_PIN(id));   
}


/**
 * setLED - turns on the LED if is_on is true, else turns it off
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
 * piDigit - returns the 0..9 value of the dig at posn
 */
int piDigit(int posn) {
   return pgm_read_byte_near(pi + posn) - '0';
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
   uint32_t now_us = micros();
   
   *newPress = false;
   *numPressed = 0;
   
   // Scan each of the buttons   
   for (digit=0; digit < NUM_BUTTONS; digit++) {
      
     // Read the state of the switch into a local variable:
     int reading = buttonPressed(digit);
   
     // Check to see if you just pressed the button
     // (i.e. the input went from false to true), and you've waited long enough
     // since the last press to ignore any noise:   
     
     // If the switch changed, due to noise or pressing:
     if (reading != buttonState[digit].lastButtonState) {
        
       // Reset the debouncing timer
       buttonState[digit].lastDebounceTime = now_us;
     }
   
     // whatever the reading is at, it's been there for longer than the debounce
     // delay, so take it as the actual current state:
     if ((now_us - buttonState[digit].lastDebounceTime) >= DEBOUNCE_DELAY) {
  
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
 * Calculate and return the current score
 */
long currentScore() {

   // 10 points for each one sequenced correctly, plus 1 point
   //   (max of 100) for those not sequenced correctly
   return (numSequenced * 10L) + min(extraNotSequenced, 100);
}


/**
 * Check to see if the optional controller is attached, and if so, use it to start and
 *     stop the match, and to display the status and score
 * The optional controller is the same one used in the 2017 SoutheastCon page and
 *     consists of a 4x20 LCD, I2C to LCD controller, and push buttons on A0 for
 *     start and A3 for stop
 */
bool controllerInstalled() {

   // Setup the button start/stop pins on the Arduino
   pinMode(BUTTON_START, INPUT_PULLUP);
   pinMode(BUTTON_STOP,  INPUT_PULLUP);

   // Check if the optional controller LCD is attached
   Wire.begin();
   Wire.beginTransmission(LCD_ADDRESS);
   bool installed = (0 == Wire.endTransmission());

   // If not, then display a status to the Arduino monitor
   if (false == installed) {
      Serial.println(F("Optional LCD controller not found"));
      Serial.println(F("Start the competition by briefly pressing any button\n"));

   // Else clear the LCD display and display start instructions
   } else {
      lcd.init();
      lcd.backlight();
      lcd.home ();

      lcd.print("SoutheastCon 2020");

      lcd.setCursor(0, 1);
      lcd.print("Do you want to...");
      lcd.setCursor(0, 2);
      lcd.print("    ...play a game?");

      lcd.setCursor(0,3);
      lcd.print(" vvv Press to START");
      Serial.println(F("LCD controller FOUND - press the START button to begin"));
   }

   return installed;
}


/**
 * Returns true if either start on the optional controller or any of the 10
 *    pi buttons have been pressed
 */
bool matchStart() {

   bool started = false;

   // If started on optional controller, display stop instructions now
   if (hasController && (!digitalRead(BUTTON_START))) {
      started = true;
   }

   // If start not pressed, quickly scan to see if any of the 10 pi buttons
   //    have been pressed - we are NOT worrying about debouncing for start
   int button;
   for (button=0; button < NUM_BUTTONS; button++) {
       if (buttonPressed(button)) {
          while (buttonPressed(button));
          started = true;
       }
    }

    // If started and controller present, display stop directions
    if (hasController && started) {
       lcd.setCursor(0, 1);
       lcd.print("                    ");
       lcd.setCursor(0, 2);
       lcd.print("                    ");

       lcd.setCursor(0,3);
       lcd.print("Press to STOP  vvv ");      
    }
    
    // Neither start or a button pressed, so not starting yet
    return started;
}


/**
 *  Returns true if the A3 on the optional controller is pressed (no way to
 *     stop the contest if running from a laptop for testing purposes
 */
bool matchStop() {
   return (hasController and (!digitalRead(BUTTON_STOP)));
}


/**
 * Update the score on the controller LCD display
  */
void updateController(uint32_t runTime) {

      lcd.setCursor(0, 1);
      lcd.print("TIME:  ");
      lcd.print(runTime / 1000);
      lcd.print("     ");
      
      lcd.setCursor(13, 1);
      lcd.print("S:");
      lcd.print(numSequenced);
      lcd.print("    ");

      lcd.setCursor(0, 2);
      lcd.print("SCORE: ");
      lcd.print(currentScore());
      lcd.print("     ");

      lcd.setCursor(13, 2);
      lcd.print("X:");
      lcd.print(extraNotSequenced);
      lcd.print("    ");
}
 
 /**
 * Do whatever set up is required to wait for the competition to start,
 *    and to display the first digit (add a little LED bling of bouncing
 *    LED lights while waiting for the contest to start
 */
void startCompetition() {

   uint32_t ledDelay = 500;      // for a 2hz rate
   uint32_t start = millis();    // start of delay for each led dance
   int led = 1;                  // where to start the dance
   int direction = -1;           // initial direction of the light dance
   
   // The judge starts the competition by pressing any one button
   // For fun, do a bouncing LED back and forth until the button press
   while (!matchStart()) {

      // Is it time to move to the next LED?
      if ((millis() - start) > ledDelay) {
 
         // If so, turn the previous one off, advance, turn next on
         start = millis();
         setLED(led, false);
         led += direction;
         setLED(led, true);
 
         // Is it time to reverse direction?
         if ((led == 0) or (led == 9)) {
            direction = -direction;
         }
      }
   }

   // Quick flash to indicate the competition has begun
   setAllLEDs(true);
   delay(FLASH_INTERVAL);
   setAllLEDs(false);

   // Pause a bit to make sure judge lets go of the start button... ;-)
   delay(100);
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
   Serial.print(F("Num sequenced correctly:"));
   Serial.println(numSequenced);
   Serial.print(F("Extra not sequenced correctly:"));
   Serial.println(extraNotSequenced);

   // Display the final score
   Serial.print(F("\nFinal score: "));
   Serial.println(currentScore());

   if (hasController) {
      lcd.setCursor(0,3);
      lcd.print("RESET for next game");
   }

   
   // Wait here forever...
   for(;;);
}


/**
 * setup - this is executed once at the start of the sketch, so perform any
 *    necessary setup operations here
 */
void setup() {

   int i;
   
   Serial.begin(115200);
   Serial.println(HELLO);
   Serial.println(VERSION);
   delay(50);
   
   // Setup our buttons as input and our leds as output, and turn the LEDs off
   for (i=0; i < NUM_BUTTONS; i++) {
      pinMode(LED_PIN(i), OUTPUT);
      pinMode(BUTTON_PIN(i), INPUT_PULLUP);
      setLED(i, false);
   }

   // Check if the optional LCD/button controller is installed
   hasController = controllerInstalled();

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

   uint32_t now = millis();
   uint32_t runTime = now - startTimestamp;
   boolean newPress;
   int numPressed;

   // Update optional controller every second
   if (hasController && ((runTime % 1000) == 0)) {
      updateController(runTime);
   }

   // If the match is over, stop and report the score now and halt here
   if ((runTime >= MATCH_RUNTIME) || (matchStop())) {
      endCompetition();  // Will not return from this
   }

   // If we are 'flashing' all LEDs, see if the flash on interval is over
   if ((flashTimeout != 0) && (now >= flashTimeout)) {
      setAllLEDs(false);
   }
   
   // Debounce all the buttons...   
   debounceButtons(&newPress, &numPressed);
   
   // If all buttons are released, then light up the next button in the sequence
   if (numPressed == 0) {
      if (inSequence) {
         setLED(piDigit(piDigitPosn), true);
      }
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
        int digit = piDigit(piDigitPosn);

        // If the wrong button was pressed, we are no longer sequencing
        if (buttonState[digit].buttonState == false) {
           inSequence = false;
           extraNotSequenced++;
           flashAllLEDs();

        // Else new press, correct so turn off digit, increment count of correct
        //   digits and light up the next digit
        } else {
           setLED(digit, false);
           piDigitPosn++;
           numSequenced = piDigitPosn;
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

