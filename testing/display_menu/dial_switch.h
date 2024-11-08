#ifndef DIAL_SWITCH_H
#define DIAL_SWITCH_H

#include "dial_switch.h"
#include <Arduino.h>

const int DIAL_STEPS = 12;
const int VOLTAGE_BOUNDARY = (int)(1024/dial_steps - 2);

class Dial_Switch {
  private:
    int analog_pin;
    int dial_position; // 
  public:
    Dial_Switch(int analog_pin) {
      this->dial_position = 0; // set dial_position to zero initially to see if errors occur
      this->analog_pin = analog_pin;
    }

    void read_adc() {
      volatile int sensorValue = analogRead(analog_pin);

      if(!sensorValue) {
        Serial.print("Error reading ADC from Pin: "); 
        Serial.println(analog_pin);
        return;
      }
      
      for(int i=12; i*voltage_boundary > 0; i--) {
        if(sensorValue >= (i*voltage_boundary)) {
          int dial_position = 13-i;
          // Update the object's instance of the dial position equal to the calculated dial position
          this->dial_position = dial_position;
        }
      }
    }

    int get_dial_pos() {
      return dial_position; // same as this->dial_position
    }

};
#endif