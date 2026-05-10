/*************************************************** 
  This is an example for our Adafruit 24-channel PWM/LED driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1429

  These drivers uses SPI to communicate, 3 pins are required to  
  interface: Data, Clock and Latch. The boards are chainable

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution

  Fixed typo in all occurences of NUM_TLC5947 (was 5974)
 ****************************************************/

#include "Adafruit_TLC5947.h"

// How many boards do you have chained?
#define NUM_TLC5947 1

#define data   4
#define clock   5
#define latch   6
#define oe  0  // set to -1 to not use the enable pin (its optional)

const int buttonPin = 10;

const int totalLEDs = 4;

int numPips = 1;
bool buttonLatch = false;
int ownerColor = 0;

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5947, clock, data, latch);

uint16_t Red[3] = {4095, 0, 0};
uint16_t Off[3] = {0, 0, 0};

uint16_t targetHSB[3] = {0,4095,4095};
uint16_t trueHSB[3] = {0, 4095, 4095};
uint16_t HSBtoRGB[3];

void setup() {

  //take initial color read
  hsb_to_rgb(targetHSB, HSBtoRGB);

  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);
  
  Serial.println("TLC5947 test");
  tlc.begin();
  if (oe >= 0) {
    pinMode(oe, OUTPUT);
    digitalWrite(oe, LOW);
  }
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  if(buttonState == LOW && !buttonLatch) {
    numPips++;

    if(numPips > 4) {numPips = 1; ownerColor++;}
    if(ownerColor > 3) {ownerColor = 0;}

    buttonLatch = true;
  }
  else if(buttonState == HIGH) {
    buttonLatch = false;
  }
  Serial.print("TargetColor: ");
  Serial.println(ownerColor * 90);
  Serial.print("RGB value ");
  Serial.print(HSBtoRGB[0]);
  Serial.print(", ");
  Serial.print(HSBtoRGB[1]);
  Serial.print(", ");
  Serial.println(HSBtoRGB[2]);

  targetHSB[0] = ownerColor * 90;

  trueHSB[0] = lerp_hue(trueHSB[0], targetHSB[0], 0.05f);

  hsb_to_rgb(trueHSB, HSBtoRGB);

  for(int i = 0; i < totalLEDs; i++)
  if(i < numPips) {
    LEDWrite(i, HSBtoRGB);
  }
  else {
    LEDWrite(i, Off);
  }
}

#include <stdint.h>

#define HSB_MAX 4095

//basic HSB to RGB converter for easier color shifting
void hsb_to_rgb(const uint16_t hsb[3], uint16_t rgb[3])
{
    uint16_t h = hsb[0] % 360;
    uint16_t s = hsb[1];
    uint16_t v = hsb[2];

    if (s == 0) {
        rgb[0] = v;
        rgb[1] = v;
        rgb[2] = v;
        return;
    }

    uint16_t region = h / 60;
    uint16_t remainder = h % 60;

    uint32_t p = ((uint32_t)v * (HSB_MAX - s)) / HSB_MAX;
    uint32_t q = ((uint32_t)v * (HSB_MAX - ((uint32_t)s * remainder) / 60)) / HSB_MAX;
    uint32_t t = ((uint32_t)v * (HSB_MAX - ((uint32_t)s * (60 - remainder)) / 60)) / HSB_MAX;

    switch (region) {
        case 0:
            rgb[0] = v;
            rgb[1] = t;
            rgb[2] = p;
            break;

        case 1:
            rgb[0] = q;
            rgb[1] = v;
            rgb[2] = p;
            break;

        case 2:
            rgb[0] = p;
            rgb[1] = v;
            rgb[2] = t;
            break;

        case 3:
            rgb[0] = p;
            rgb[1] = q;
            rgb[2] = v;
            break;

        case 4:
            rgb[0] = t;
            rgb[1] = p;
            rgb[2] = v;
            break;

        default:
            rgb[0] = v;
            rgb[1] = p;
            rgb[2] = q;
            break;
    }
}

uint16_t lerp_u16(uint16_t a, uint16_t b, float t)
{
    return (uint16_t)(a + (b - a) * t + 0.5f);
}

uint16_t lerp_hue(uint16_t from, uint16_t to, float t)
{
    int16_t start = from % 360;
    int16_t end = to % 360;

    int16_t diff = end - start;

    if (diff > 180) {
        diff -= 360;
    } else if (diff < -180) {
        diff += 360;
    }

    if (diff == 0) {
        return end;
    }

    int16_t step = (int16_t)(diff * t);

    if (step == 0) {
        step = (diff > 0) ? 1 : -1;
    }

    int16_t result = start + step;

    if (result < 0) {
        result += 360;
    } else if (result >= 360) {
        result -= 360;
    }

    return (uint16_t)result;
}

void LEDWrite(uint16_t i, uint16_t *rgb) {
  tlc.setLED(i, rgb[0], rgb[1], rgb[2]);
  tlc.write();
}
