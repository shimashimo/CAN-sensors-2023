```CPP
#include "Adafruit_LiquidCrystal.h"
//Global Variables
//For switch case, calculated in interrupt
int Rotary_output_value = 0;
int dial_steps = 4;

//Will Need to know which pins are used to init LCD
Adafruit_LiquidCrystal lcd();

void setup()
{
  Serial.begin(115200);

  //Rows & Columns Init
  lcd.begin(20,2);

  lcd.print("Vroom Vroom.");
  delay(500);
  lcd.noDisplay();
  delay(500);
}

void loop()
{
  
}

---------------------------------------

void  KMH_Gear_RPM()
{
    // 13 Characters: Km/Hr: ##
    lcd.print("Km/Hr: ");
    lcd.print(speed);
    lcd.print("    ");

    // 7 characters: Gear: #
    lcd.print("Gear: ");
    lcd.print(gear);

    // Set Cursor to row 1, col 0
    lcd.setCursor(0,1);

    //13 characters: RPM: # X1000
    lcd.print("RPM: ");
    lcd.print(RPM);
    lcd.print(" X1000");

  }

void   Charge_and_brake_display()
{
    // 11 Characters: Charge: ##%
    lcd.print("Charge: ");
    lcd.print(charge);
    lcd.print("%");
    lcd.print(" ");

    // Set Cursor to row 1, col 0
    lcd.setCursor(0,1);

    //20 characters: Brake Pressure: ## Pa
    lcd.print("Brake Pressure: ");
    lcd.print(pressure);
    lcd.print("Pa");

  }

void  GLV_display()
{
    // Should be in V, mV? should be double?
    lcd.print("GLV Voltage: ");
    lcd.print(volts);
    lcd.print("V")

    // Set Cursor to row 1, col 0
    lcd.setCursor(0,1);

    // Should be in A, mA? should be double?
    lcd.print("GLV Current: ");
    lcd.print(Current);
    lcd.print("A");


}

void  Temp_display()
{
    // 17 Characters: Engine Temp: ###C
    lcd.print("Engine Temp: ");
    lcd.print(engT);
    lcd.print("C")

    // Set Cursor to row 1, col 0
    lcd.setCursor(0,1);

    // 17 Characters: Acmltr Temp: ###C
    lcd.print("Acmltr Temp: ");
    lcd.print(acct);
    lcd.print("C");


}

void display_state()
{
  int Rotary_output_value;
  int dial_steps = 12; // even voltage interval over ## of dial positions
  int voltage_bndry = (int)(1024 / dial_steps); // 1024 / 12 = 85.33 (the interval with which the voltage increases -> [i * 85v] )

  // loop steps through each interval and checks if the analog voltage read is within the range.
  // checking smallest interval to largest allows us to check first if our value is greater than
  // a boundary value, if it is, then increases 'i'. If we find our voltage is less than a boundary
  // value, then 'i' is our found Rotary_output_value.
  for (int i=1; i * voltage_bndry < 1024; i++)
  {
    if (AnalogRead(ROTARY_SWITCH_PIN) <= (i * voltage_bndry))
    {
      Rotary_output_value = i;
      break; // we have found and set the value, now exit the loop
    }
  }

  // now instead, our switch case starts at 1 instead of 0
  switch (Rotary_output_value)
  {
    case 1:
      // menu display 1 ...

    case 2:
      // menu display 2 ...

    case ##:
      // other subsequent display menus ...
  }
}

------------------------------------



```
