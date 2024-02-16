#include <LiquidCrystal.h>
#include <FastLED.h>

#define LED_PIN     7
#define NUM_LEDS    13
#define DELAY_TIME  220
#define BRIGHTNESS 100

CRGB leds[NUM_LEDS];

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

  RPMtoLED( 0, RPM );

  // leds[0] = CRGB::Lime;
  // leds[1] = CRGB::Lime;
  // leds[2] = CRGB::Lime;
  // leds[3] = CRGB::Lime;
  // leds[4] = CRGB::Lime;
  // leds[5] = CRGB::Red;
  // leds[6] = CRGB::Red;
  // leds[7] = CRGB::Red;
  // leds[8] = CRGB::Red;
  // leds[9] = CRGB::RoyalBlue;
  // leds[10] = CRGB::RoyalBlue;
  // leds[11] = CRGB::RoyalBlue;
  // leds[12] = CRGB::RoyalBlue;

  FastLED.show();
}


// RPM range is from 0 - 14000, therefore each light represents ~1077 rpm
void RPMtoLED( unsigned char section, unsigned int RPM) {

  switch(section){
    case 0:
      // Could set each led based on 1 range of values, but the sensor may not
      // pick up the RPM value before it sets the next light (i.e could skip a light). 
      // Would be safer to set all prev leds?

      if (RPM <= 1077) leds[0] = CRGB::Lime;
      if (RPM > 1077 && RPM <= 2154) leds[1] = CRGB::Lime;
      if (RPM > 2154 && RPM <= 3231) leds[2] = CRGB::Lime;
      if (RPM > 3231 && RPM <= 4308) leds[3] = CRGB::Lime;
      if (RPM > 4308 && RPM <= 5385) leds[4] = CRGB::Lime;
      break;


    // case 1078 ... 2154:
    //   for(int i=0; i<NUM_LEDS; i++){
    //     leds[i] = CRGB::Black;
    //   }

    //   leds[0] = CRGB::Lime;
    //   leds[1] = CRGB::Lime;
    //   leds[2] = CRGB::Lime;
    //   leds[3] = CRGB::Lime;
    //   leds[4] = CRGB::Lime;
    //   leds[5] = CRGB::Red;
    //   leds[6] = CRGB::Red;
    //   leds[7] = CRGB::Red;
    //   leds[8] = CRGB::Red;
    //   break;
    default:
      Serial.println("Fuck");
      break;
  }
  
}
