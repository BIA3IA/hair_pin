#pragma once
#include <stdint.h>
constexpr int OUTPUT = 1, HIGH = 1, ADC_11db = 3;
unsigned long millis();
uint32_t analogReadMilliVolts(uint8_t pin);
void pinMode(uint8_t pin, int mode);
void digitalWrite(uint8_t pin, int value);
void analogReadResolution(int bits);
void analogSetPinAttenuation(uint8_t pin, int attenuation);
