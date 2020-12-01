
// Chute - no bluetooth required!
// Joel Adria
// Based on RemoteLEDSender ICSC Example
// v1.1
// Aug 23, 2017


// /Users/joeladria/Dropbox/Work/Nova\ Digital\ Parachute/Code/uploadAll.sh /Users/joeladria/Documents/Arduino/Chute-2017-aug23/Chute-2017-aug23.ino.with_bootloader.micro.hex

// Wind chimes: 280.3 Hz, 396.4 Hz, 420,0 Hz 529.1, 560.6


#include <Wire.h>
#include <SPI.h>
#include <ICSC.h>

#include <EEPROM.h>


#include <anyrtttl.h>
#include <binrtttl.h>
#include <pitches.h>

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

const char * twinkle = "Twinkle:d=4,o=5,b=80:32p,16p,16c,16p,16c,16p,16g,16p,16g,16p,16a,16p,16a,16p,4g,16f,16p,16f,16p,16e,16p,16e,16p,16d,16p,16g,16p,16c6";

int emajor[12] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, NOTE_E6}; // 12 notes
int echroma[12] = {659, 698, 740, 784, 831, 880, 932, 988, 1047, 1109, 1175, 1245}; // 12 notes

int powerTimer = 12;

#include <FastLED.h>
FASTLED_USING_NAMESPACE
#define FRAMES_PER_SECOND  120
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define LED_PIN     6
#define NUM_LEDS    12
#define BRIGHTNESS  255
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

uint8_t chuteID = EEPROM.read(0);

ICSC icsc(Serial1, chuteID, 5);
static unsigned long ts = millis();

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

bool powerOn = 1;

void setup()
{

  Serial.println(chuteID);

  delay(1000); // 1 second delay for recovery

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  // for ChuteNet
  Serial1.begin(19200);

  // for debug
  Serial.begin(9600);

  icsc.begin();
  icsc.registerCommand('A', &modeChange);
  icsc.registerCommand('B', &modeChange);
  icsc.registerCommand('C', &modeChange);
  icsc.registerCommand('D', &modeChange);
  icsc.registerCommand('E', &modeChange);
  icsc.registerCommand('F', &modeChange);
  icsc.registerCommand('G', &modeChange);
  icsc.registerCommand('H', &modeChange);
  icsc.registerCommand('I', &modeChange);  

  icsc.registerCommand('T', &modeChange);
  icsc.registerCommand('P', &modeChange);
  icsc.registerCommand('X', &speedChange);

  // accel start
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldn't connect to accel");
    while (1) {
      tone(9, 200);
      delay(1000);
      noTone(9);
      delay(1000);
    }
  }

  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  // ready to go
//  tone(9, NOTE_B5, 100);
//  delay(100);
//  tone(9, NOTE_E6, 850);
//  delay(800);
//  noTone(9);


}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {
  test,       // T 0
  accelGame,  // A 1 -- charge up colours
  ocean,      // B 2 -- white dots jumping out from handles
  mushroom,   // C 3 -- bursts from centre
  pinwheel,   // D 4 -- rainbow rotate
  fireworks,  // E 5 -- spiral formation
  starfield,  // F 6 -- 
  rainbow,    // G 7 -- rainbow from middle
  stix,   // H 8
  confetti,       // I 9
  powerOff,   // poweroff 10
};


//

uint8_t gCurrentPatternNumber = 6; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
float accelVariation;
uint8_t maxForce = 0;
uint8_t localTimer = 0;

float runningAverage = 0;

bool latch;
bool holdDown;
static unsigned long latchTimer = millis();
static unsigned long sensorTimer = millis();
static unsigned long fxTimer = millis();
uint8_t latchTimeout = 250; // msM

uint8_t fxSpeed = 255; // value from 0-255, controllable from RGB thingy?

sensors_event_t event;

void loop()
{

  // 1. process networking
  icsc.process();

  // 4. process local sensor
  if ((millis() - sensorTimer) > 100) {

    lis.getEvent(&event);
    sensorTimer = millis();
  }


  /* Display the results (acceleration is measured in m/s^2) */
  //
  //  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
  //  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
  //  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
  //  Serial.println(" m/s^2 ");

  // check sum of all motion
  float sum = abs(event.acceleration.z) + abs(event.acceleration.y) + abs(event.acceleration.x);

  // see if our sum is deviating from the average
  accelVariation = abs(runningAverage - sum);
  //  Serial.print(" difference from average: ");
  //  Serial.print(accelVariation);

  // calculate the average afterwards to avoid skewing the result
  runningAverage = (runningAverage + sum) / 2;
  //  Serial.print("runningAverage: ");
  //  Serial.print(runningAverage);

  // see latch if we're in the threshold. maybe needs a timeout vs an average-out?
  //  Serial.print((millis() - latchTimer));
  if ((millis() - latchTimer) < latchTimeout) {
    holdDown = 1;
  } else {
    holdDown = 0;
  }
  if (accelVariation > 5 && holdDown != 1) {
    // leave on for 18ms -> this should work itself out with runningAverage, then keep off for 500ms
    latch =  1;
    latchTimer = millis();

  }

  //  Serial.print(" latch: ");
  //  Serial.println(latch);


  // 6. update LEDs (every 17ms = 60fps)
  EVERY_N_MILLISECONDS(17) {
    gPatterns[gCurrentPatternNumber]();
    
    // keep a blue led on all the time to keep power
    // this could probably use some logic (if LED sum is less than, then don't do this) But this is safer...
    if (powerOn) {
      leds[0] |= CRGB( 0, 0, 255);
    }
    FastLED.show();
    latch = 0;
  }


  EVERY_N_MILLISECONDS( 50 ) {
    localTimer++;

    gHue = localTimer;


  }

  // 7. update piezo
  anyrtttl::nonblocking::play();


}


void clockSync(char *data, unsigned char len) {
//  Serial.print(" Incoming timer:");
//  for (uint8_t i = 0; i < len; i++) {
//    Serial.print(data[i]);
//  }
//
//  Serial.print(" Casting:");
  int intTimer = atoi(data);
//  Serial.print(intTimer);
//
//  Serial.print(" Local timer:");
//  Serial.print(localTimer);
//
//  int delta = localTimer - intTimer;
//  Serial.print(" Variance:");
//  Serial.println(delta);

  localTimer = intTimer;
}


int headingStrip;
int progressTimer = 0;

void modeChange(unsigned char src, char command, unsigned char len, char *data) {

  Serial.print("Packet from ");
  Serial.print(src);
  Serial.print("Received MODE signal");

  switch (command) {
    case 'P':
      gCurrentPatternNumber = 10;
      Serial.println("Powering off...");
      break;

    case 'T':
      powerTimer = 12;
      clockSync(data, len);
      gCurrentPatternNumber = 0;
      tone(9, NOTE_A3);
      delay(10);
      noTone(9);
      if ((localTimer % 10) + 1 == chuteID) {

      }
      Serial.println("In test mode...");
      break;

    // compass mode
    case 'D':
      
      if (gCurrentPatternNumber != (int(command) - 64)) {
        tone(9, NOTE_A3);
        delay(30);
        noTone(9);
      }
      gCurrentPatternNumber = int(command) - 64;
      Serial.print(" Current mode: ");
      Serial.println(gCurrentPatternNumber);
      headingStrip = atoi(data);
      break;

    case 'B': // accelGame/Mushroom
      // if we change modes, then we want to go into normal "accel" mode with no clock.
      if (gCurrentPatternNumber != (int(command) - 64)) {
        gHue = 0;

      }
      progressTimer = atoi(data);

      clockSync(data, len);

      gCurrentPatternNumber = int(command) - 64;
      Serial.print(" Current mode: ");
      Serial.println(gCurrentPatternNumber);
      Serial.print(" Progres timer: ");
      Serial.println(progressTimer);
      break;


    default:
      // a = 1, b = 2, etc... http://www.kerryr.net/pioneers/ascii2.htm

      clockSync(data, len);
      if (gCurrentPatternNumber != (int(command) - 64)) {
        powerTimer = 12;
        tone(9, NOTE_A3);
        delay(30);
        noTone(9);
        if (command == 'F') { // play twinkle twinkle
          //          anyrtttl::nonblocking::begin(9, twinkle);
        }
      }
      gCurrentPatternNumber = int(command) - 64;
      Serial.print(" Current mode: ");
      Serial.println(gCurrentPatternNumber);
      break;

  }
  //  stats_t *duck = icsc.stats();
  //
  //  Serial.print("TX Fail:");
  //  Serial.print(duck->tx_fail);
  //  Serial.print(" CS Errors:");
  //  Serial.print(duck->tx_bytes);
  //  Serial.print(" TX Bytes:");
  //  Serial.print(duck->tx_bytes);
  //  Serial.print(" RX Packs:");
  //  Serial.print(duck->rx_packets);
  //  Serial.print(" RX Bytes:");
  //  Serial.print(duck->rx_bytes);
  //  Serial.print(" CS Errors:");
  //  Serial.print(duck->cs_errors);
  //  Serial.print(" CS Run:");
  //  Serial.print(duck->cb_run);
  //  Serial.print(" CS Bad:");
  //  Serial.print(duck->cb_bad);
  //  Serial.print(" Collision:");
  //  Serial.print(duck->collision);
  //  Serial.print(" OOB:");
  //  Serial.println(duck->oob_bytes);

}


void speedChange(unsigned char src, char command, unsigned char len, char *data) {
  Serial.print(" Incoming speed:");
  for (uint8_t i = 0; i < len; i++) {
    Serial.print(data[i]);
  }

  Serial.print(" Casting:");
  fxSpeed = atoi(data);
  Serial.print(fxSpeed);
}
