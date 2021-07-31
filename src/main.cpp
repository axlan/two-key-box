#include <Arduino.h>
#include <FastLED.h>

// 6 total pins
// 0 is P0, 1 is P1, 4 is P4 - unlike the analog inputs,
// for analog (PWM) outputs the pin number matches the port number.
#define PIN_HALL_LEFT 2
#define PIN_HALL_RIGHT 3
// On Digispark clones Pin 5 may not be useable without changing
// a fuse to switch it from a reset pin to GPIO.
#define PIN_RELAY 5
// P0, P1, and P4 are capable of hardware PWM (analogWrite).
#define PIN_LEDS_RED 0
#define PIN_LEDS_BLUE 1
#define PIN_LEDS_GREEN 4

#define SLEEP_PERIOD_MS 50
#define UNLOCK_PERIODS 2

int unlock_count = 0;
uint8_t pattern_index = 0;

enum {
  BOTH = 0b00,
  LEFT = 0b01,
  RIGHT = 0b10,
  NONE = 0b11
};

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LEDS_RED, OUTPUT);
  pinMode(PIN_LEDS_BLUE, OUTPUT);
  pinMode(PIN_LEDS_GREEN, OUTPUT);
  pinMode(PIN_HALL_LEFT, INPUT);
  pinMode(PIN_HALL_RIGHT, INPUT);
}

CHSV ColorFade(uint8_t hue, uint8_t idx) {
  CHSV color(hue ,255, idx);
  if (idx > 127) {
    color.val = 255 - idx;
  }
  // Avoid the flash from the LED going fully off
  color.val = max(color.val, 1);
  return color;
}

uint8_t ReadSensors() {
  uint8_t tmp = digitalRead(PIN_HALL_LEFT) << 1;
  return tmp | digitalRead(PIN_HALL_RIGHT);
}

void loop() {

  uint8_t sensor_state = ReadSensors();

  CHSV color;
  switch (sensor_state)
  {
  case NONE:
    color = ColorFade(pattern_index, pattern_index);
    break;
  case LEFT:
    color = ColorFade(HUE_PURPLE, pattern_index * 2);
    break;
  case RIGHT:
    color = ColorFade(HUE_YELLOW, pattern_index * 2);
    break;
  case BOTH:
    color = CHSV(HUE_GREEN, 255, 255);
    break;
  default:
    color = CHSV(HUE_RED, 255, 255);
  }

  if (sensor_state != BOTH) {
    unlock_count = 0;
  }

  if (sensor_state != BOTH || unlock_count >= UNLOCK_PERIODS) {
    digitalWrite(PIN_RELAY, LOW);
  } else {
    digitalWrite(PIN_RELAY, HIGH);
    unlock_count++;
  }

  CRGB rgb;
  hsv2rgb_rainbow(color, rgb);
  analogWrite(PIN_LEDS_RED, rgb.red);
  analogWrite(PIN_LEDS_GREEN, rgb.green);
  analogWrite(PIN_LEDS_BLUE, rgb.blue);

  pattern_index++;
  delay(SLEEP_PERIOD_MS);
}
