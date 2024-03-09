#include "led_tachometer.h"

const int BRIGHTNESS = 5;
int RPM = 1000;

LED_Tachometer led(BRIGHTNESS);

void setup() {
  Serial.begin(9600);
  led.begin();
}

void loop() {
  led.setLEDs( RPM );
  FastLED.show();
  RPM += 50;
}

