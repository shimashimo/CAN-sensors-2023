/*
  Modified code of ReadAnalogVoltage example and dial_switch_case_concept.md

  Kinda jank but enough for testing the dial switches

  Wanted to put in a redundancy check for the for loop in get_dial_pos() function
  but requires global vars and wasn't sure how to do that atm. Leaving as a TODO task
*/
#include "dial_switch.h"
const int DRIVE_DIAL = A0;
const int MENU_DIAL = A1;
Dial_Switch drive_dial(DRIVE_DIAL_PIN);
Dial_Switch menu_dial(MENU_DIAL_PIN);


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  // Read in the dial position (integer)
  drive_dial.read_adc();
  menu_dial.read_adc();

  // Serial.println(drive_pos);
  if(!drive_dial.get_dial_pos() || !menu_dial.get_dial_pos()) {
    Serial.println("Dial Switch Error");
    return 0;
  }

  // Use dial position for switch cases
  drive_switch_case(drive_pos);
  menu_switch_case(menu_pos);

}
