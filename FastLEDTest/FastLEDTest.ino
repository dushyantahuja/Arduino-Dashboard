
#include "FastLED.h"


#define DATA_PIN    A5
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    53
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  1

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i< NUM_LEDS; i++){
    int R = random(255);
    int G = random(255);
    int B = random(255);
    leds[i] = CRGB(R,G,B);
  }
  FastLED.show();    
}

void loop() {
  // put your main code here, to run repeatedly:

}
