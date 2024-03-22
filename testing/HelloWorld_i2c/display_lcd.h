#ifndef DISPLAY_LCD_H
#define DISPLAY_LCD_H

#include "Adafruit_LiquidCrystal.h"

#define UPRIGHT 1
#define UPLEFT 2
#define DOWNLEFT 3
#define DOWNRIGHT 4
#define UNIT_OFFSET 6


/*
  The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * CLK to Analog #5
 * DAT to Analog #4
*/

void menu_setup(Adafruit_LiquidCrystal &lcd);
void update_section_int(Adafruit_LiquidCrystal &lcd, int section, int menuNum, float newValue );
void clear_section_int(Adafruit_LiquidCrystal &lcd, int section);

extern char menu_units[10][4][100];

#endif