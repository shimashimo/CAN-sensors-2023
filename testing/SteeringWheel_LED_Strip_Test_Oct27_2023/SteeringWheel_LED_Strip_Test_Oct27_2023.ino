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
  14000,
};

void setLEDs();

void setup() {

  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  FastLED.setBrightness( 10 );  // Set global brightness of the LEDs
  // for(int i=0; i<NUM_LEDS; i++){
  //   leds[i] = CRGB::Black;
  // }
  Serial.begin( 9600 );
  Serial.println("Enter RPM: ");
}

void loop() {

  while (Serial.available() == 0) {
    
  }

  unsigned int RPM = Serial.parseInt();
  Serial.println("RPM Entered: " + (String)RPM);

  setLEDs( RPM );

  FastLED.show();
}


// RPM range is from 0 - 14000, therefore each light represents ~1077 rpm
void setLEDs( unsigned int RPM) {

  int led_index; // Led to start changing color of in the loop
  int section;  // Led to stop changing color at in the loop
  CRGB LED_color;


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
    fill_color(leds, NUM_LEDS, CRGB::Magenta);
    fill_color(leds, NUM_LEDS, CRGB::Black);
  }

  for(int i=led_index; i < section+1; i++) {
    if(RPM <= RPM_THRESHOLDS[i]) {
      
      // Sets all the previous lights to the LED_color ** Necessary?
      // Is there a case where the RPM would skip threshold levels in between sensor readings?
      // for(int j=0; j < i; j++) {
      //   leds[j] = LED_color;
      // }


      leds[i] = LED_color;
      Serial.println(i);
      clearOtherLEDs(i+1);
      break; // Break at first threshold RPM is lower than
    }
  }
  
}

void clearOtherLEDs(int i){
  for (i; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
}