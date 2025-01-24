#ifndef DIAL_SWITCH_H
#define DIAL_SWITCH_H

#include <Arduino.h>

/*
  Simple dial switch class. 
  DIAL_STEPS = how many "positions" the dial has.  
  This example uses voltage boundaries to map analogRead( ) to a discrete position.
*/

#define DIAL_STEPS 6

class Dial_Switch {
public:
  Dial_Switch(int pin) : _pin(pin), _dialPos(1), _prevDialPos(1), _changed(false) {
    pinMode(_pin, INPUT);
  }

  void read_adc() {
    int val = analogRead(_pin);
    // Map 0-1023 to 1-6 with clear boundaries
    int newPos;
    
    if (val < 170)        newPos = 1;
    else if (val < 340)   newPos = 2;
    else if (val < 510)   newPos = 3;
    else if (val < 680)   newPos = 4;
    else if (val < 850)   newPos = 5;
    else                  newPos = 6;

    if (newPos != _dialPos) {
      _dialPos = newPos;
      _changed = true;
      _prevDialPos = _dialPos;
    }
  }

  int get_dial_pos() const {
    return _dialPos;
  }

  // Return true if dial position changed since last check
  bool check_change() const {
    return _changed;
  }

  // Reset the "changed" flag after you handle it
  void reset_change() {
    _changed = false;
  }

private:
  int _pin;
  int _dialPos;
  int _prevDialPos;
  volatile bool _changed;
};

#endif // DIAL_SWITCH_H
