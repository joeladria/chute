// These are basically all the "mode" function calls
// So any game logic should be built into these, including
// passing any "timing" functions from over the network I guess

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}
//
//
//void standby() {
//  // alternate amber and blue
//  // maybe we should make it "twinkle" instead as the standby?
//  if ((gHue % 8) == 0) {
//
//    // odd
//    for (uint8_t i = 0; i < NUM_LEDS - 1; i++) {
//      leds[i] = CRGB::PaleGoldenrod;
//      leds[i + 1] =  CRGB::LightSteelBlue;
//    }
//  }
//  else {
//    // even
//    for (uint8_t i = 0; i < NUM_LEDS - 1; i++) {
//      leds[i + 1] = CRGB::PapayaWhip;
//      leds[i] =  CRGB::LightSteelBlue;
//    }
//  }
//}


/// accel game

uint8_t maxLevel = 0;
int gameTimer = 0;
void accelGame() {
  // THE BUILD UP
  if (maxLevel < 12 ) {
    if (latch) {
      tone(9, emajor[maxLevel], 300);
      delay(50);
      noTone(9);
      maxLevel++;
      gameTimer = 0;
    } else {
      gameTimer++;
      if (gameTimer > 15 && maxLevel > 0) {
        maxLevel--;
        tone(9, emajor[maxLevel], 300);
        delay(50);
        noTone(9);
        gameTimer = 0;
      }
    }

    fill_solid( leds, 12, CRGB(98, 46, 50));
    if (maxLevel > 12) {
      maxLevel = 12;
    }

    for (int i = 0; i < maxLevel; i++) {
      leds[i] = CRGB(0, 0, 255);
    }
  }


}


void mushroom() {
  //fill_solid(leds, 12, CHSV(292, 100, 40));
//  if (gHue > 21  && gHue < 33) {
    uint8_t toLight = -(gHue % 11) + 11;
    leds[toLight] = CHSV( gHue,200, 255); // random shade of purple
//  }
  nscale8( leds, NUM_LEDS, 240); // slow fade out

  // base colour
  for (uint8_t i = 0; i< NUM_LEDS; i++) {
    leds[i] |= CHSV(292, 100, 40); 
  }

}


void pinwheel() {
    // fill the others are deterministic color
    uint8_t hueResult = (gHue * 2) + map(chuteID, 0, 12, 0, 255);
    fill_solid( leds, 12, CHSV(hueResult , 200, 255));
}

void fireworks() {
  
  // ghue = colour of the explosion
  // more of a spiral, not fireworks
  // create a blink as it progresses outwards -- every cycle we alternate "value" from 0 to 1
  fill_solid( leds, 12, CRGB::Black);


  //  Spiral 1
  if (gHue % 12 == chuteID) {
    uint8_t toLight = constrain(map(gHue, 0, 255, 11, 0), 0, 11);
    leds[toLight] = CHSV(gHue, 255, 255); // random shade of green
  }

  // Spiral 2
  uint8_t gHueAlt = gHue + 120;
  if (gHueAlt % 12 == chuteID) {
    uint8_t toLight = constrain(map(gHueAlt, 0, 255, 11, 0), 0, 11);
    leds[toLight] = CHSV(gHue * 2, 255, 255); // random shade green
  }

    // base colour
  for (uint8_t i = 0; i< NUM_LEDS; i++) {
    leds[i] |= CRGB(0, 20, 0); 
  }
  //  if ( (millis() - (fxTimer)) > fxSpeed) {
  //

  //    //uint8_t toLight = -(gHue % 11) + 11;
  //    uint8_t toLight = map(((gHue + (chuteID * 21)), 0, 512, 11, 0);
  //                          Serial.println(toLight);
  //
  //                          fxTimer = millis();
  //
  //
  //
  //  }


  //nscale8_video( leds, NUM_LEDS, 10); // not quite to black?


}


void ocean() {

  if ( millis() - (fxTimer) > 50) {

    // shuffle everything in torwards the middle, but only once and a while
    for (uint8_t i = NUM_LEDS - 1; i > 0; i--) {
      leds[i] = leds[i - 1];
    }
    fxTimer = millis();
  }

  if (latch) {
    leds[0] = CRGB::White;
    
    tone(9, NOTE_G4, 35);
    delay(35);
    tone(9, NOTE_C4, 35);
    delay(35);
    tone(9, NOTE_D4, 35);
    delay(35);
    noTone(9);

  } else {
    leds[0] = CRGB::Blue;
  }

}


void starfield () {
  // this is more of a "warp speed" type effect

  if ( millis() - (fxTimer) > fxSpeed) {

    int randomStar = random8(chuteID, 255);
    if ((randomStar % 2) == 0) {
      randomStar = 0;
    }
    leds[NUM_LEDS - 1] = CRGB(randomStar, randomStar, randomStar);
    for (uint8_t i = 0; i < NUM_LEDS - 1; i++) {
      leds[i] = leds[i + 1];
    }
    fxTimer = millis();
  }

}




void sparkle() {
  // random colours sparkles

  int      twinkrate = 100;                                     // The higher the value, the lower the number of twinkles.
  uint8_t  thisdelay =  10;                                     // A delay value for the sequence(s).
  uint8_t   thisfade =   5;                                     // How quickly does it fade? Lower = slower fade rate.
  uint8_t    thishue =  50;                                     // The hue.
  uint8_t    thissat = 100;                                     // The saturation, where 255 = brilliant colours.
  uint8_t    thisbri = 255;                                     // Brightness of a sequence.
  bool       randhue =   1;                                     // Do we want random colours all the time? 1 = yes.

  if (twinkrate < NUM_LEDS) twinkrate = NUM_LEDS;             // Makes sure the twinkrate will cover ALL of the LED's as it's used as the maximum LED index value.
  int i = random16(twinkrate + chuteID);                              // A random number based on twinkrate. Higher number => fewer twinkles.
  if (randhue) thishue = random16(0, 255);                    // Randomize every LED if TRUE
  if (i < NUM_LEDS) leds[i] = CHSV(thishue, thissat, thisbri); // Only the lowest probability twinkles will do. You could even randomize the hue/saturation.
  for (int j = 0; j < NUM_LEDS; j++) leds[j].fadeLightBy(thisfade); // Use the FastLED fade method.


}




void test() {
  // strand ID
  if (gHue > 0 && gHue < 50) {
    fill_solid( leds, 12, CRGB::Black);
    fill_solid( leds, chuteID, CRGB(255, 255, 255));
  }

  // for sync
  if (gHue > 50 && gHue < 99) {
    fill_solid( leds, 12, CRGB::Black);
    fill_solid( leds, (gHue % 12), CRGB(120, 120, 120));
  }

  if (gHue > 100 && gHue < 149) {
    fill_solid( leds, 12, CRGB::Black);
    fill_solid( leds, 12, CRGB(255, 0, 0));
  }

  if (gHue > 150 && gHue < 199) {
    fill_solid( leds, 12, CRGB::Black);
    fill_solid( leds, 12, CRGB(0, 255, 0));
  }
  if (gHue > 200 && gHue < 255) {
    fill_solid( leds, 12, CRGB::Black);
    fill_solid( leds, 12, CRGB(0, 0, 255));
  }
}


void powerOff() {
  if ( (millis() - (fxTimer)) > 1000) {
    if (powerOn) {
      powerTimer--;
      tone(9, 200);
      delay (100);
      noTone(9);
    }
    fxTimer = millis();
  }

  fill_solid( leds, 12, CRGB::Black);
  for (int i = 0; i < powerTimer; i++) {
    fill_solid( leds, i, CRGB::White);
  }
  Serial.println(powerTimer);
  if (powerTimer < 1) {

    powerOn = 0;
  }


}


void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue*4, 7);
}


//
//void juggle() {
//  // eight colored dots, weaving in and out of sync with each other
//  fadeToBlackBy( leds, NUM_LEDS, 20);
//  byte dothue = 0;
//  for ( int i = 0; i < 8; i++) {
//    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
//    dothue += 32;
//  }
//}

void confetti()
{

  fadeToBlackBy( leds, NUM_LEDS, 10);

  if ( millis() - (fxTimer) > fxSpeed) {

  // random colored speckles that blink in and fade smoothly
  
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);

    fxTimer = millis();
  }


}
//
void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}


// 1/6, 2/7, 3/8, 4/9, 5/10

void stix() {
  
  

  uint8_t fast = gHue*8;
  uint8_t angle = map(fast, 0, 255, 1, 11);
  if (chuteID == angle ) {
    fill_solid( leds, 12, CHSV(gHue, 150, 220));
  } 
  else { 
      fadeToBlackBy( leds, NUM_LEDS, 20);
  }
  

}



//
//void bpm()
//{
//  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
//  uint8_t BeatsPerMinute = 62;
//  CRGBPalette16 palette = PartyColors_p;
//  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
//  for ( int i = 0; i < NUM_LEDS; i++) { //9948
//    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
//  }
//}


