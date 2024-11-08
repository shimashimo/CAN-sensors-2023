#ifndef DIAL_SWITCH_H
#define DIAL_SWITCH_H
#include <Arduino.h>
#include "US2066.h"


int get_dial_pos(int ROTARY_SWTICH_PIN);
void drive_switch_case(int drive_pos, US2066 OLED); 
void menu_switch_case(int menu_pos, US2066 OLED);

#endif