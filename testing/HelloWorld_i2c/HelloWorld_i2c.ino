/*
 Demonstration sketch for Adafruit i2c/SPI LCD backpack
 using MCP23008 I2C expander
 ( https://learn.adafruit.com/i2c-spi-lcd-backpack )

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * CLK to Analog #5
 * DAT to Analog #4
*/

// include the library code:
#include "Adafruit_LiquidCrystal.h"
#include "display_lcd.h"

// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

// float values[4];

void setup() {
  Serial.begin(115200);
  // while(!Serial);
  Serial.println("LCD Character Backpack I2C Test.");

  // set up the LCD's number of rows and columns:
  // NOTE: can be tested on our 20 x 4 display at Q-hut. Change (16, 2) -> (20, 4)
  if (!lcd.begin(20, 4)) { 
    Serial.println("Could not init backpack. Check wiring.");
    while(1);
  }
  Serial.println("Backpack init'd.");

  // Print a message to the LCD.
  // lcd.print("hello, world!");
  menu_setup(lcd);
}

void loop() {
  // for(int i=0; i<4; i++){
  //   values[]
  // }
  // update_all(lcd, values, 1);
  update_section_int(lcd, UPRIGHT, 1, millis()/1000);
  update_section_int(lcd, UPLEFT, 1, 1+millis()/1000);
  update_section_int(lcd, DOWNLEFT, 1, 3+millis()/1000);
  update_section_int(lcd, DOWNRIGHT, 1, 4+millis()/1000);

  // delay(1000);
} 

