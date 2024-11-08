/*
  Library for determining Dial Switch positions. Can use the switch cases here with includes for OLED display
  or if its easier implement them in the display or main file, define the functions there and pass in the Dial Position.
*/
#include "dial_switch.h"

int dial_steps = 12;
int voltage_boundary = (int)(1024/dial_steps - 2);  // 1024 / 12 = 85.33 (the interval with which the voltage increases -> [i * 85v] )


int get_dial_pos(int ROTARY_SWTICH_PIN)
{
  // read the input on analog pin 0:
  // Analog reading goes from 0 - 1023
  int sensorValue = analogRead(ROTARY_SWTICH_PIN);

  /* Can probably do this better but idk rn */
    // Reversing as voltage is greatest at position 1
    for(int i=12; i*voltage_boundary > 0; i--) {
      if(sensorValue >= (i*voltage_boundary)) {
        int dial_position = 13-i;
        return dial_position;
      }
    }

    return 0; // Error if returning zero
}

void drive_switch_case(int dial_position, US2066 OLED) 
{
  /* IMPLEMENT DRIVE MODE STUFF HERE */
  switch (dial_position){
    case 1:
      OLED.print("Drive Mode");
      delay(4000);
      break;
      
    case 2:
      OLED.print("Eco Mode");
      delay(1000);
      // Serial.println("ECO Mode");
      break;
      
    case 3:
      Serial.println("3rd Mode");
      break;

    case 4:
      Serial.println("4th Mode");
      break;

    default:
      Serial.println("Default");
      break;
  }
}

void menu_switch_case(int dial_position, US2066 OLED)
{
  /* IMPLEMENT DISPLAY MENU STUFF HERE */
  switch (dial_position){
    case 1:
      // det_drive_mode(Some Data); Determine drive mode then update screen
      
      OLED.print(0, 0, " 25 KMH");
      OLED.print("  :  ");
      OLED.print("1000 RPM");
      break;
      
    case 2:
      OLED.print(0, 0, " ## V");
      OLED.print("  :  ");
      OLED.print(" ## A");
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
