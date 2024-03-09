#include "Arduino.h"
#include "led_tachometer.h"


const int NUM_LEDS = 13;

// Index for RPM threshold values
// The upper bound for the RPM to be that color using RPM_THRESHOLDS[ COLOR ]
// I.E The lights will be green when RPM < 9693, where GREEN_SEC = 4
const int GREEN_SEC = 4;
const int YELLOW_SEC = 0;

CRGB leds[NUM_LEDS];

// Each LED represents 1077 RPM
// Coordinate the green section with the nominal RPM [Setting Green < 10770]
// Yellow section is 
const int RPM_THRESHOLDS[NUM_LEDS] = { 
  13500, 12824,
  11847, 10770,
  9693, 8616,
  7539, 6462,
  5385, 4308,
  3231, 2154,
  1077,
};

// Vars and Consts for blinking without delay
bool redlineLED = false; // State variable of LED flash for redline
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 100;  // interval at which to blink (milliseconds)


void LED_Tachometer::begin()
{
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness( m_BRIGHTNESS );  // Set global brightness of the LEDs
}


/*
  Function: Turns leds on and color codes based on RPM.
            First 8 LEDs are green,
            Last 4 LEDs are yellow
            Redline flashes when above 13500 RPM
            RPM range is from 0 - 14000, therefore each light represents range of ~1077 rpm
*/
void LED_Tachometer::setLEDs( unsigned int RPM )
{
  unsigned long currentMillis = millis(); // Check to see if its time to blink redline

  int led_index; // Led to start changing color of in the loop
  int section;  // Led to stop changing color at in the loop
  CRGB LED_color;

  // Categorize RPM into the 2 colors
  if(RPM <= RPM_THRESHOLDS[ GREEN_SEC ]) {
    led_index = NUM_LEDS;
    section = GREEN_SEC;
    LED_color = CRGB::Lime;
  }
  if(RPM > RPM_THRESHOLDS[ GREEN_SEC ] && RPM <= RPM_THRESHOLDS[ YELLOW_SEC ]) {
    led_index = GREEN_SEC - 1;
    section = YELLOW_SEC;
    LED_color = CRGB::Yellow;
  }
  if(RPM > RPM_THRESHOLDS[ YELLOW_SEC ]) { // Hitting Redline RPM - BAD
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; // save the last time we blinked the LED

      if(redlineLED) {
        // Red Lights are on, so we want them off
        fill_solid(leds, NUM_LEDS, CRGB::Black); 
      } else {
        fill_solid(leds, NUM_LEDS, CRGB::Red); // Vice versa - red lights off, so we want them on
      }
      FastLED.show(); // Update physical LED strip to show color
      redlineLED = !redlineLED; // Toggle red LED state variable
      // global_RPM = 0;
      return;
    }
  }

  // Loop that turns on the leds
  for(int i=led_index; i >= section; i--) {
    if(RPM <= RPM_THRESHOLDS[i]) {
      
      // Sets all the previous lights to the LED_color ** Necessary?
      // Is there a case where the RPM would skip threshold levels in between sensor readings?
      // for(int j=led_index; j < i; j++) {
      //   leds[j] = LED_color;
      // }

      leds[i] = LED_color;
      clearOtherLEDs(i-1);
      // if (RPM >= RPM_THRESHOLDS[ YELLOW_SEC ]) {                 // Uncomment for testing with rising RPM
      //   // global_RPM = 0;
      // }
      break; // Break at the first threshold RPM is lower than
    }
  }
}

void LED_Tachometer::clearOtherLEDs(int i){
  for (i; i >= 0; i--){
    leds[i] = CRGB::Black;
  }
}


