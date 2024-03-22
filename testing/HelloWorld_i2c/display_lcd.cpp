#include "display_lcd.h"

char menu_units[10][4][100]= {
  {"KMH", "V", "", "°C"},     // Speed, State of Charge, Drive Mode, Engine Temp
  {"A", "", "V", "°C"},
};

void menu_setup(Adafruit_LiquidCrystal &lcd) {
  lcd.setCursor(9, 0);
  lcd.print("|");
  lcd.setCursor(9, 1);
  lcd.print("|");
  lcd.setCursor(9, 2);
  lcd.print("|");
  lcd.setCursor(9, 3);
  lcd.print("|");
}
/**
 * Updates all values
 * 
 * @param lcd Adafruit_LiquidCrystal LCD object instance 
 * @param section Portion of the display to update (UpRight, UpLeft, DownLeft, DownRight)
 * @param menuNum Selected menu option to display
 * @param newValueArr New values to display in the section
 */ 
void update_all(Adafruit_LiquidCrystal &lcd, float newValueArr[], int menuNum)
{
  int columns[] = {9, 0, 0, 9};
  int line = 0;
  char units[100] = "NON";

  for(int quad=0; quad < 4; quad++){
    if(quad > 1) line = 1;
    lcd.setCursor(columns[quad], line);
    lcd.print(newValueArr[quad]);

    strcpy(units, menu_units[menuNum][quad][0]);
    lcd.setCursor(columns[quad+UNIT_OFFSET], line);
    lcd.print(units);

  }
}


/**
 * Updates values in an individual quadrant
 * 
 * @param lcd Adafruit_LiquidCrystal LCD object instance 
 * @param section Portion of the display to update (UpRight, UpLeft, DownLeft, DownRight)
 * @param menuNum Selected menu option to display
 * @param newValue New value to display in the section
 */          
void update_section_int(Adafruit_LiquidCrystal &lcd, int section, int menuNum, float newValue )
{
  int col = 0;
  int line = 0;
  char units[100] = "NON";

  // Change cursor position based on desired section and menu number
  switch( section ) {
    case 1: 
      // set the cursor to column 8, line 0
      col = 11;
      line = 0;

      switch( menuNum ){
        case 1:
          strcpy(units, "KMH");
      }

      break;
    case 2:
      // set the cursor to column 0, line 0
      col = 0;
      line = 0;
      break;
    case 3:
      col = 0;
      line = 2;
      break;
    case 4:
      col = 11;
      line = 2;
      break;
  }

  lcd.setCursor(col, line);

  if( newValue ) {
    lcd.print( newValue, 1 );
  }
  lcd.setCursor(col+6, line);
  lcd.print(units);

}

void clear_section_int(Adafruit_LiquidCrystal &lcd, int section)
{
  int col;
  int line;
  // Change cursor position based on desired section and menu number
  switch( section ) {
    case 1: 
      // set the cursor to column 8, line 0
      col = 11;
      line = 0;
      break;
    case 2:
      // set the cursor to column 0, line 0
      col = 0;
      line = 0;
      break;
    case 3:
      col = 0;
      line = 2;
      break;
    case 4:
      col = 11;
      line = 2;
      break;
  }

  lcd.setCursor(col, line);
  lcd.print("    ");
}

// void update_section_string(int section, char[] string)
// {

// }

