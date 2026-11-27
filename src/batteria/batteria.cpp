#include "batteria.h"
#include "../ble/protocollo.h"
#include <Arduino.h>

namespace {
constexpr uint8_t BAT_ADC_PIN = 0, BAT_EN_PIN = 15;
constexpr unsigned long SAMPLE_INTERVAL_MS = 30000;
constexpr unsigned SAMPLE_COUNT = 16;
// Curva indicativa per una LiPo 1S; non sostituisce un fuel gauge.
constexpr uint16_t MILLIVOLTS[] = {3300, 3500, 3600, 3700, 3750, 3800, 3850, 3900, 4000, 4100, 4200};
constexpr uint8_t PERCENT[] = {0, 5, 10, 20, 30, 40, 50, 60, 75, 90, 100};
unsigned long last_sample = 0;
uint32_t filtered_mv = 0;
uint8_t percentage = BATTERY_UNKNOWN;

void sample() {
  uint32_t total = 0;
  for (unsigned i = 0; i < SAMPLE_COUNT; ++i) total += analogReadMilliVolts(BAT_ADC_PIN);
  uint32_t mv = total * 3 / SAMPLE_COUNT;
  if (mv < 2500 || mv > 4350) {
    percentage = BATTERY_UNKNOWN;
    filtered_mv = 0;
    return;
  }
  filtered_mv = filtered_mv ? (3 * filtered_mv + mv) / 4 : mv;
  percentage = 100;
  if (filtered_mv <= MILLIVOLTS[0]) percentage = 0;
  else for (unsigned i = 1; i < sizeof(PERCENT); ++i) {
    if (filtered_mv <= MILLIVOLTS[i]) {
      percentage = PERCENT[i - 1] + (filtered_mv - MILLIVOLTS[i - 1]) *
          (PERCENT[i] - PERCENT[i - 1]) / (MILLIVOLTS[i] - MILLIVOLTS[i - 1]);
      break;
    }
  }
}
} // namespace

void batteria_init() {
  pinMode(BAT_EN_PIN, OUTPUT);
  digitalWrite(BAT_EN_PIN, HIGH);
  analogReadResolution(12);
  analogSetPinAttenuation(BAT_ADC_PIN, ADC_11db);
  filtered_mv = 0;
  sample();
  last_sample = millis();
}

void batteria_update() {
  if (millis() - last_sample < SAMPLE_INTERVAL_MS) return;
  last_sample = millis();
  sample();
}

uint8_t batteria_percentuale() { return percentage; }
