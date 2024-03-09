#include "led_tachometer.h"

const int BRIGHTNESS = 5;
#define GREEN_SEC 4
#define YELLOW_SEC 0
int RPM = 1000;

LED_Tachometer led(BRIGHTNESS, GREEN_SEC, YELLOW_SEC);

void setup() {
  Serial.begin(9600);
  led.begin();
}

void loop() {
  led.setLEDs( RPM );
  FastLED.show();
  RPM += 50;
}

