/*
  Modified code of ReadAnalogVoltage example and dial_switch_case_concept.md

  Kinda jank but enough for testing the dial switches

  Wanted to put in a redundancy check for the for loop in get_dial_pos() function
  but requires global vars and wasn't sure how to do that atm. Leaving as a TODO task
*/
#include "dial_switch.h"
int DRIVE_DIAL = A0;
int MENU_DIAL = A1;


void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  // Get the dial position (number)
  int drive_pos = get_dial_pos(DRIVE_DIAL);
  int menu_pos = get_dial_pos(MENU_DIAL);

  // Serial.println(drive_pos);
  if(!drive_pos || !menu_pos) {
    Serial.println("Dial Switch Error");
    return 0;
  }

  // Use dial position for switch cases
  drive_switch_case(drive_pos);
  menu_switch_case(menu_pos);

}
