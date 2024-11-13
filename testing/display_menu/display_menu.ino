/*
  This file is for testing the OLED display with the dial switches using a switch case to change
  menu screens. This has been copied into the main steering wheel canduino file as of Nov.11 2024.
*/

#include <Arduino.h>
#include "dial_switch.h"
#include "US2066.h"

US2066 OLED;

const int DRIVE_DIAL = A0;
const int MENU_DIAL = A1;
Dial_Switch drive_dial(DRIVE_DIAL_PIN);
Dial_Switch menu_dial(MENU_DIAL_PIN);

int get_dial_pos(int ROTARY_SWTICH_PIN);
void menu_switch_case(int dial_position, US2066 OLED)
unsigned long pretend_value;

// Keep track of last menu displayed, if different clear to display next menu
int last_menu_pos;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  OLED.init();
}

void loop() {
  pretend_value = random(0, 1000);  // Pretend value for testing
  // Read from ADC to get dial position
  drive_dial.read_adc();
  menu_dial.read_adc();
  // Store dial pos in variables
  int drive_pos = drive_dial.get_dial_pos();
  int menu_pos = menu_dial.get_dial_pos();

  if(!drive_pos || !menu_pos) {
    Serial.println("Dial Switch Error");
    return 1;
  }

  if(menu_pos != last_menu_pos) {
    OLED.clear();
    last_menu_pos = menu_pos;
  }
  menu_switch_case(menu_pos, OLED);
  // drive_switch_case(drive_pos);
  delay(300);
}



/* IMPLEMENT DISPLAY MENU STUFF HERE */
void menu_switch_case(int dial_position, US2066 OLED)
{
  switch (dial_position){
    case 1:
      // det_drive_mode(DriveModeData); /** Determine drive mode then update screen **/
      char SpeedBuf[8];
      char RPMbuf[8];
      char TempBuf[8];
      OLED.print(0, 0, "Speed:");
      itoa(pretend_value, SpeedBuf, 10);   // Requires itoa() as the oled print function uses String type
      OLED.print(0, 9, SpeedBuf);

      OLED.print(1, 0, "SOCa: ");
      itoa(pretend_value+123, RPMbuf, 10);
      OLED.print(1, 9, RPMbuf);
      
      OLED.print(2, 0, "Drive: ");
      if(pretend_value % 2) OLED.print(2, 9, "ECO  ");   // Just for testing with changing values
      else OLED.print(2, 9, "ACCEL");                 // just for testing, replace with Drive Mode data

      OLED.print(3, 0, "ENGTemp:");
      itoa(pretend_value-88, TempBuf, 10);
      OLED.print(3,9, TempBuf);
      break;
      
    case 2:
      OLED.print(0, 0, "SOC: ");
      OLED.print(1, 0, " ## A");
      break;
      
    case 3:
      Serial.println("Accumulator Voltages");
      break;

    case 4:
      Serial.println("Accumulator Temperatures");
      break;

    case 5:
      Serial.println("Brake Pressure");
      break;

    case 6:
      Serial.println("Suspension Travel");
      break;

    default:
      Serial.println("Default");
      break;
  }
}