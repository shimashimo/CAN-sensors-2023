#ifndef led_strip_h
#define led_strip_h

#include "Arduino.h"

#include <LiquidCrystal.h>
#include <FastLED.h>

class LED_Tachometer {
  public:
    LED_Tachometer(int brightness) : m_BRIGHTNESS(brightness) {}
    void begin();
    void setLEDs( unsigned int RPM);
    void clearOtherLEDs(int i);

  private:
    static constexpr int LED_PIN = 7;
    int m_BRIGHTNESS;
};

#endif