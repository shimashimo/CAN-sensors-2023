#include <LiquidCrystal.h>
#include <FastLED.h>

#define LED_PIN     7
#define NUM_LEDS    13
#define DELAY_TIME  220
#define BRIGHTNESS 100

const int GREEN_SEC = 4;
const int RED_SEC = 8;
const int BLUE_SEC = 12;

CRGB leds[NUM_LEDS];

const int RPM_THRESHOLDS[NUM_LEDS] = {
  1077, 2154,
  3231, 4308,
  5385, 6462,
  7539, 8616,
  9693, 10770,
  11847, 12824,
  13500,
};

unsigned int global_RPM = 0;


void setLEDs();

void setup() {

  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  FastLED.setBrightness( 5 );  // Set global brightness of the LEDs
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
}


/*
  Function: Turns leds on and color codes based on RPM.
            First 5 LEDs are green,
            Next 4 LEDs are red,
            Last 4 LEDs are blue
            Redline flashes when above 13500 RPM
            RPM range is from 0 - 14000, therefore each light represents range of ~1077 rpm
*/
void setLEDs( unsigned int RPM) {

  int led_index; // Led to start changing color of in the loop
  int section;  // Led to stop changing color at in the loop
  CRGB LED_color;

  // Categorize RPM into the 3 colors
  if(RPM <= RPM_THRESHOLDS[ GREEN_SEC ]) {
    led_index = 0;
    section = GREEN_SEC;
    LED_color = CRGB::Lime;
  }
  if(RPM > RPM_THRESHOLDS[ GREEN_SEC ] && RPM <= RPM_THRESHOLDS[ RED_SEC ]) {
    led_index = GREEN_SEC + 1;
    section = RED_SEC;
    LED_color = CRGB::Red;
  }
  if(RPM > RPM_THRESHOLDS[ RED_SEC ] && RPM <= RPM_THRESHOLDS[ BLUE_SEC ]) {
    led_index = RED_SEC + 1;
    section = BLUE_SEC;
    LED_color = CRGB::RoyalBlue;
  }
  if(RPM > RPM_THRESHOLDS[ BLUE_SEC ]) { // Redlining

    // Flash Lights when redline RPM
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(100);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(100);
    global_RPM = 0;
    return;
  }


  for(int i=led_index; i < section+1; i++) {
    if(RPM <= RPM_THRESHOLDS[i]) {
      
      // Sets all the previous lights to the LED_color ** Necessary?
      // Is there a case where the RPM would skip threshold levels in between sensor readings?
      // for(int j=led_index; j < i; j++) {
      //   leds[j] = LED_color;
      // }

      leds[i] = LED_color;
      clearOtherLEDs(i+1);
      if (RPM >= RPM_THRESHOLDS[ BLUE_SEC - 1]) {                 // Uncomment for testing with rising RPM
        global_RPM = 0;
      }
      break; // Break at the first threshold RPM is lower than
    }
  }
  
}

void clearOtherLEDs(int i){
  for (i; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
}