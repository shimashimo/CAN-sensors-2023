#ifndef DIAL_SWITCH_H
#define DIAL_SWITCH_H

#include "dial_switch.h"
#include <Arduino.h>

const int DIAL_STEPS = 12;
const int VOLTAGE_BOUNDARY = (int)(1024/dial_steps - 2);

class Dial_Switch {
  private:
    int analog_pin;
    int dial_position;
    int last_dial_position;
    bool dial_change;
    int last_voltage;
  public:
    Dial_Switch(int analog_pin) {
      this->analog_pin = analog_pin;
      this->dial_position = 0; // set dial_position to zero initially to see if errors occur
      this->last_dial_position = 0;
      this->dial_change = false;
      this->last_voltage = false;
    }

    void read_adc() {
      volatile int sensorValue = analogRead(analog_pin);

      if(!sensorValue) {
        Serial.print("Error reading ADC from Pin: "); 
        Serial.println(analog_pin);
        return;
      }
      
      // Check if the ADC reading is different than last time. If different, check dial position
      // reduces need to check the dial position every time. +-1 for ADC margin of error.
      if(sensorValue > last_voltage+1 || sensorValue < last_voltage - 1) {
        int dial_position;
        for(int i=DIAL_STEPS; i*voltage_boundary > 0; i--) {
          if(sensorValue >= (i*voltage_boundary)) {
            dial_position = DIAL_STEPS+1 - i;   // Hack way of doing this - but works (sorry)
            // Update the object's instance of the dial position equal to the calculated dial position
            this->dial_position = dial_position;
          }
        }
        // double check that the dial position changed - not just voltage fluctuation
        if(this->dial_position != last_dial_position) {
          dial_change = true;
          last_dial_position = this->dial_position;
        }
      }
      last_voltage = sensorValue;
    }

    int get_dial_pos() {
      return dial_position; // same as this->dial_position
    }

    bool check_change() {
      return dial_change;
    }
    void reset_change() {
      dial_change = false;
    }
};
#endif