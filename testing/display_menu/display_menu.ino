#include "dial_switch.h"
US2066 OLED;

int DRIVE_DIAL = A0;
int MENU_DIAL = A1;

unsigned char wheelSpeed[8];


int menu_pos_buff;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  OLED.init();
}

void loop() {
  // Get the dial position (number)
  int drive_pos = get_dial_pos(DRIVE_DIAL);
  int menu_pos = get_dial_pos(MENU_DIAL);

  if(!drive_pos || !menu_pos) {
    Serial.println("Dial Switch Error");
  }

  // clear when switching menus
  if(menu_pos != menu_pos_buff) {
    OLED.clear();
  }
  menu_pos_buff = menu_pos;
  menu_switch_case(menu_pos, OLED);
  // drive_switch_case(drive_pos, OLED);

}