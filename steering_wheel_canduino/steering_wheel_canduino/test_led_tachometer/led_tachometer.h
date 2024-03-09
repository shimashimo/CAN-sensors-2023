#ifndef led_strip_h
#define led_strip_h

#include "Arduino.h"

#include <LiquidCrystal.h>
#include <FastLED.h>

class LED_Tachometer {
  public:
    LED_Tachometer(int brightness, int GREEN_SEC, int YELLOW_SEC) : m_BRIGHTNESS(brightness),  m_GREEN_SEC(GREEN_SEC), m_YELLOW_SEC(YELLOW_SEC) {}
    void begin();
    void setLEDs( unsigned int RPM);
    void clearOtherLEDs(int i);

  private:
    static constexpr int LED_PIN = 7;
    int m_BRIGHTNESS;
    const int m_GREEN_SEC;
    const int m_YELLOW_SEC;
};

#endif