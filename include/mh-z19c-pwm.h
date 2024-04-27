#pragma once

#include <Arduino.h>

template <uint8_t pin>
class MH_Z19C_PWM {
 public:
  void begin() {
    pinMode(pin, INPUT);
    attachInterrupt(pin, onPWMEdgeCanged, CHANGE);
  }

  uint16_t getPPM() {
    return MH_Z19C_PWM::ppm;
  }

 private:
  static volatile int64_t lastRiseTime;
  static volatile int64_t lastFallTime;
  static volatile uint16_t ppm;

  static void onPWMEdgeCanged();
};

template <uint8_t pin>
volatile int64_t MH_Z19C_PWM<pin>::lastRiseTime = 0;
template <uint8_t pin>
volatile int64_t MH_Z19C_PWM<pin>::lastFallTime = 0;
template <uint8_t pin>
volatile uint16_t MH_Z19C_PWM<pin>::ppm = 0;

template <uint8_t pin>
void MH_Z19C_PWM<pin>::onPWMEdgeCanged() {
  int64_t now = micros();

  if (digitalRead(pin)) {
    int64_t t    = now - lastRiseTime;
    int64_t th   = lastFallTime - lastRiseTime;
    ppm          = (uint16_t)(5000.0 * (th / 1000.0 - 2.0) / (t / 1000.0 - 4.0));
    lastRiseTime = now;
  } else {
    lastFallTime = now;
  }
}
