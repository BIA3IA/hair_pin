#include "imu/cadenza.h"
#include <unity.h>

static unsigned long clock_ms = 0;
unsigned long millis() { return clock_ms; }

void setUp() {
  clock_ms += 5000;
  is_walking(false, false);
  is_walking(true, false); // primo passo dopo una pausa
}
void tearDown() {}

static bool step(unsigned long interval, bool rotating = false) {
  clock_ms += interval;
  return is_walking(true, rotating);
}

static void starts_on_third_regular_step() {
  TEST_ASSERT_FALSE(step(600));
  TEST_ASSERT_TRUE(step(600));
}

static void slow_walk_starts_on_third_step() {
  TEST_ASSERT_FALSE(step(850));
  TEST_ASSERT_TRUE(step(850));
}

static void rejects_irregular_steps() {
  TEST_ASSERT_FALSE(step(600));
  TEST_ASSERT_FALSE(step(1000));
  TEST_ASSERT_FALSE(step(600));
}

static void ignores_rotation() {
  TEST_ASSERT_FALSE(step(600, true));
  TEST_ASSERT_FALSE(step(600, true));
  TEST_ASSERT_FALSE(step(600)); // troppo distante dal primo
}

static void stop_and_restart_need_a_new_rhythm() {
  step(600);
  step(600);
  step(600);
  clock_ms += 1220;
  TEST_ASSERT_FALSE(is_walking(false, false));
  TEST_ASSERT_FALSE(step(600));
  TEST_ASSERT_FALSE(step(600));
  TEST_ASSERT_TRUE(step(600));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(starts_on_third_regular_step);
  RUN_TEST(slow_walk_starts_on_third_step);
  RUN_TEST(rejects_irregular_steps);
  RUN_TEST(ignores_rotation);
  RUN_TEST(stop_and_restart_need_a_new_rhythm);
  return UNITY_END();
}
