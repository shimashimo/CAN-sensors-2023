#ifndef DIAL_SWITCH_H
#define DIAL_SWITCH_H
#include <Arduino.h>

int get_dial_pos(int ROTARY_SWTICH_PIN);
void drive_switch_case(int dial_position); 
void menu_switch_case(int dial_position);

#endif