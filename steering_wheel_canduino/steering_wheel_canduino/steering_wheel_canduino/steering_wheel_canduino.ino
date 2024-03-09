#include <LiquidCrystal.h>
#include <FastLED.h>

#define LED_PIN     7
#define NUM_LEDS    13
#define BRIGHTNESS 5

// Index for RPM threshold values
// The upper bound for the RPM to be that color using RPM_THRESHOLDS[ COLOR ]
// I.E In the green zone, the RPM will be less than RPM_THRESHOLDS[ GREEN_SEC ] == 11847 rpm, where GREEN_SEC = 10
const int GREEN_SEC = 8;
const int YELLOW_SEC = 12;

CRGB leds[NUM_LEDS];

// Each LED represents 1077 RPM
// Coordinate the green section with the nominal RPM [Setting Green < 10770]
// Yellow section is 
const int RPM_THRESHOLDS[NUM_LEDS] = { 
  1077, 2154,
  3231, 4308,
  5385, 6462,
  7539, 8616,
  9693, 10770,
  11847, 12824,
  13500,
};


// Vars and Consts for blinking without delay
bool redlineLED = false; // State variable of LED flash for redline
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 100;  // interval at which to blink (milliseconds)

unsigned int global_RPM = 0;


void setLEDs();

void setup() {

  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  FastLED.setBrightness( BRIGHTNESS );  // Set global brightness of the LEDs
  Serial.begin( 9600 );
  // Serial.println("Enter RPM: ");      // Uncomment for testing with User input RPM
}

void loop() {

  // while (Serial.available() == 0) {}          // Uncomment for testing with User input RPM
  // unsigned int RPM = Serial.parseInt();       // Uncomment for testing with User input RPM
  // Serial.println("RPM: " + (String)RPM);   // Uncomment for testing with User input RPM
  // unsigned int RPM = 13777;

  setLEDs( global_RPM );

  FastLED.show();
  global_RPM += 50;                              // Uncomment for testing with rising RPM
  Serial.println(String(global_RPM));
}


/*
  Function: Turns leds on and color codes based on RPM.
            First 8 LEDs are green,
            Last 4 LEDs are yellow
            Redline flashes when above 13500 RPM
            RPM range is from 0 - 14000, therefore each light represents range of ~1077 rpm
*/
void setLEDs( unsigned int RPM) {

  unsigned long currentMillis = millis(); // Check to see if its time to blink redline

  int led_index; // Led to start changing color of in the loop
  int section;  // Led to stop changing color at in the loop
  CRGB LED_color;

  // Categorize RPM into the 2 colors
  if(RPM <= RPM_THRESHOLDS[ GREEN_SEC ]) {
    led_index = 0;
    section = GREEN_SEC;
    LED_color = CRGB::Lime;
  }
  if(RPM > RPM_THRESHOLDS[ GREEN_SEC ] && RPM <= RPM_THRESHOLDS[ YELLOW_SEC ]) {
    led_index = GREEN_SEC + 1;
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
  for(int i=led_index; i < section+1; i++) {
    if(RPM <= RPM_THRESHOLDS[i]) {
      
      // Sets all the previous lights to the LED_color ** Necessary?
      // Is there a case where the RPM would skip threshold levels in between sensor readings?
      // for(int j=led_index; j < i; j++) {
      //   leds[j] = LED_color;
      // }

      leds[i] = LED_color;
      clearOtherLEDs(i+1);
      // if (RPM >= RPM_THRESHOLDS[ YELLOW_SEC ]) {                 // Uncomment for testing with rising RPM
      //   // global_RPM = 0;
      // }
      break; // Break at the first threshold RPM is lower than
    }
  }
  
}

void clearOtherLEDs(int i){
  for (i; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
}