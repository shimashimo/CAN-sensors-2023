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
--------------------------------
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
  int voltage_bndry = (int)(1024 / dial_steps);
  int Rotary_input_values = analogread();

  for (int i=1; i * voltage_bndry < 1024; i++)
  {
    if (AnalogRead(ROTARY_SWITCH_PIN) <= (i * voltage_bndry))
    {
      Rotary_output_value = i;
      break; // we have found and set the value, now exit the loop
    }
  }

  switch(Rotary_output_value)
  {
    case 0:
      /*Display:

        Vehicle Speed
        Current gear
        RPM

        */
        KMH_Gear_RPM();

    case 1:
      /*Display:

      State of charge
      Brake pressure

      */
      Charge_and_brake_display();

    case 2:
      /*Display:

        GLV voltage
        GLV current

      */
      GLV_display();

    case 3:
      /*Display:

        Engine temp
        Accumulator temp

      */
      Temp_display();
  }
}

```
