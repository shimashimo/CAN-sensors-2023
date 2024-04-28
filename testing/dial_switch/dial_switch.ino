/*
  Modified code of ReadAnalogVoltage example and dial_switch_case_concept.md

  Kinda jank but enough for testing the dial switches

  Wanted to put in a redundancy check for the for loop in get_dial_pos() function
  but requires global vars and wasn't sure how to do that atm. Leaving as a TODO task
*/
#include "dial_switch.h"
int ROTARY_SWITCH_PIN = A0;


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  // Get the dial position (number)
  int dial_position = get_dial_pos(ROTARY_SWITCH_PIN);

  Serial.println(dial_position);
  if(!dial_position) {
    Serial.println("Dial Switch Error");
    return 0;
  }

  // Use dial position for switch cases
  drive_switch_case(dial_position);
  menu_switch_case(dial_position);

}
