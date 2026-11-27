#include <unity.h>
#include "batteria/batteria.h"
#include "ble/protocollo.h"
#include <stdint.h>

static unsigned long now = 0;
static uint32_t adc_mv = 0, reads = 0;
static bool power_enabled = false;
unsigned long millis() { return now; }
uint32_t analogReadMilliVolts(uint8_t pin) {
  TEST_ASSERT_EQUAL_UINT8(0, pin);
  ++reads;
  return adc_mv;
}
void pinMode(uint8_t, int) {}
void digitalWrite(uint8_t pin, int value) { power_enabled = pin == 15 && value == 1; }
void analogReadResolution(int) {}
void analogSetPinAttenuation(uint8_t, int) {}

static void estimate() {
  adc_mv = 1400;
  batteria_init();
  TEST_ASSERT_TRUE(power_enabled);
  TEST_ASSERT_EQUAL_UINT8(100, batteria_percentuale());
  adc_mv = 1300;
  batteria_init();
  TEST_ASSERT_EQUAL_UINT8(60, batteria_percentuale());
  adc_mv = 1250;
  batteria_init();
  TEST_ASSERT_EQUAL_UINT8(30, batteria_percentuale());
  adc_mv = 1000;
  batteria_init();
  TEST_ASSERT_EQUAL_UINT8(0, batteria_percentuale());
}

static void invalid_readings() {
  adc_mv = 0;
  batteria_init();
  TEST_ASSERT_EQUAL_UINT8(BATTERY_UNKNOWN, batteria_percentuale());
  adc_mv = 1500;
  batteria_init();
  TEST_ASSERT_EQUAL_UINT8(BATTERY_UNKNOWN, batteria_percentuale());
}

static void periodic_and_filtered() {
  now = 0;
  reads = 0;
  adc_mv = 1300;
  batteria_init();
  TEST_ASSERT_EQUAL_UINT32(16, reads);
  adc_mv = 1200;
  now = 29999;
  batteria_update();
  TEST_ASSERT_EQUAL_UINT32(16, reads);
  now = 30000;
  batteria_update();
  TEST_ASSERT_EQUAL_UINT32(32, reads);
  TEST_ASSERT_GREATER_THAN_UINT8(10, batteria_percentuale());
  TEST_ASSERT_LESS_THAN_UINT8(60, batteria_percentuale());
  adc_mv = 0;
  now = 60000;
  batteria_update();
  TEST_ASSERT_EQUAL_UINT8(BATTERY_UNKNOWN, batteria_percentuale());
  adc_mv = 1400;
  now = 90000;
  batteria_update();
  TEST_ASSERT_EQUAL_UINT8(100, batteria_percentuale());
}

static void timer_wraparound() {
  now = ~0UL - 10000;
  adc_mv = 1300;
  batteria_init();
  reads = 0;
  now = 19999;
  batteria_update();
  TEST_ASSERT_EQUAL_UINT32(16, reads);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(estimate);
  RUN_TEST(invalid_readings);
  RUN_TEST(periodic_and_filtered);
  RUN_TEST(timer_wraparound);
  return UNITY_END();
}
