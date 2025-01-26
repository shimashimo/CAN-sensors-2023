#ifndef DIAL_SWITCH_H
#define DIAL_SWITCH_H

#include <Arduino.h>

#define DIAL_STEPS 6
#define AVERAGE_WINDOW 15  // Number of samples to average

class Dial_Switch {
public:
  Dial_Switch(int pin) : _pin(pin), _dialPos(1), _prevDialPos(1), _changed(false) {
    pinMode(_pin, INPUT);
    // Initialize running average array
    for(int i = 0; i < AVERAGE_WINDOW; i++) {
      _samples[i] = 0;
    }
    _sampleIndex = 0;
    _sum = 0;
  }

  void read_adc() {
    int rawVal = analogRead(_pin);
    
    // Update running average
    _sum -= _samples[_sampleIndex];  // Subtract oldest sample
    _samples[_sampleIndex] = rawVal; // Store new sample
    _sum += rawVal;                  // Add new sample to sum
    _sampleIndex = (_sampleIndex + 1) % AVERAGE_WINDOW;

    // Calculate average
    int val = _sum / AVERAGE_WINDOW;

    // Map averaged value to dial positions
    int newPos;
    if (val >= 990)         newPos = 1;
    else if (val >= 890)    newPos = 2;
    else if (val >= 820)    newPos = 3;
    else if (val >= 730)    newPos = 4;
    else if (val >= 630)    newPos = 5;
    else if (val >= 530)    newPos = 6;
    else                    newPos = 0;

    if (newPos != _dialPos) {
      _dialPos = newPos;
      _changed = true;
      _prevDialPos = _dialPos;
    }
  }

  int get_dial_pos() const {
    return _dialPos;
  }

  int get_raw_average() const {
    return _sum / AVERAGE_WINDOW;
  }

  bool check_change() const {
    return _changed;
  }

  void reset_change() {
    _changed = false;
  }

private:
  int _pin;
  int _dialPos;
  int _prevDialPos;
  volatile bool _changed;
  
  // Running average variables
  int _samples[AVERAGE_WINDOW];
  int _sampleIndex;
  long _sum;
};

#endif // DIAL_SWITCH_H