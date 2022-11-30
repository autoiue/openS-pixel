
#include "FastLED.h"

#define NUM_LEDS 1

// Data pin that led data will be written out over
#define DATA_PIN 1

// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// This function sets up the ledsand tells the controller about them
void setup() {
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   //pinMode(2, OUTPUT); 
      //FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
      FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
}

// This function runs over and over, and is where you do the magic to light
// your leds.
void loop() {
  // digitalWrite(2, millis()%400>100);
   leds[0] = CHSV((millis()/20)%256, 255, (millis()/50)%256);
   leds[1] = CHSV(255, 0, (millis()/100)%256);
   FastLED.show();
   delay(10);
}