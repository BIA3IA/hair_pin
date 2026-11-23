#include "player/player.h"
#include <unity.h>
#include <vector>

static unsigned long clock_ms = 0;
static std::vector<int> frames;
unsigned long millis() { return clock_ms; }
long random(long low, long) { return low; }
int sprite_get_frame_count(const char *) { return 4; }
void sprite_draw_frame_buffered(const char *, int frame, int, int, int, int,
                               int *x, int *y, int *w, int *h) {
  frames.push_back(frame); *x = *y = 0; *w = *h = 1;
}
void setUp() { clock_ms = 0; frames.clear(); }
void tearDown() {}
static PlayerEvent advance(unsigned long amount) { clock_ms += amount; return player_update(false); }
static void forward_cycle_is_never_reversed() {
  const Clip clip = {"complete", PlayMode::FORWARD, 0, 0, 0, 100};
  player_start(&clip);
  for (int i = 0; i < 3; i++) TEST_ASSERT_EQUAL((int)PlayerEvent::NONE, (int)advance(100));
  TEST_ASSERT_EQUAL((int)PlayerEvent::DONE, (int)advance(100));
  TEST_ASSERT_EQUAL_UINT(4, frames.size());
  for (int i = 0; i < 4; i++) TEST_ASSERT_EQUAL(i, frames[i]);
}
static void walking_finishes_forward_even_on_quick_stop() {
  const Clip clip = {"walking", PlayMode::LOOP, 0, 0, 0, 100};
  player_start(&clip); advance(100); player_stop(StopStyle::QUICK);
  advance(100); advance(100);
  TEST_ASSERT_EQUAL((int)PlayerEvent::DONE, (int)advance(100));
  TEST_ASSERT_EQUAL_UINT(4, frames.size());
  for (int i = 0; i < 4; i++) TEST_ASSERT_EQUAL(i, frames[i]);
}
// inclinata: ondeggia finche' serve (mai ferma) e poi si rialza fino alla posa base
static void tilt_keeps_swaying_and_gets_back_up() {
  const Clip clip = {"balance", PlayMode::HOLD, 2, 0, 0, 100};
  player_start(&clip);
  for (int i = 0; i < 3; i++) advance(100);
  TEST_ASSERT_TRUE(player_holding());
  advance(180); TEST_ASSERT_EQUAL(2, player_frame());
  advance(180); TEST_ASSERT_EQUAL(3, player_frame());
  advance(5000); TEST_ASSERT_TRUE(player_holding());
  player_stop(StopStyle::QUICK);
  PlayerEvent event = PlayerEvent::NONE;
  for (int i = 0; i < 10 && event != PlayerEvent::DONE; i++) event = advance(50);
  TEST_ASSERT_EQUAL((int)PlayerEvent::DONE, (int)event);
  TEST_ASSERT_EQUAL(0, frames.back());
}
static void interrupted_forward_action_never_replays_backwards() {
  const Clip clip = {"coffee", PlayMode::FORWARD, 0, 0, 0, 100};
  player_start(&clip); advance(100); player_stop(StopStyle::QUICK);
  TEST_ASSERT_EQUAL((int)PlayerEvent::DONE, (int)advance(100));
  TEST_ASSERT_EQUAL_UINT(2, frames.size());
  TEST_ASSERT_EQUAL(1, frames.back());
}
static void tired_pose_waits_before_getting_up() {
  const Clip clip = {"tired", PlayMode::HOLD, 1, 2000, 2000, 100};
  player_start(&clip); advance(200); advance(200); advance(200);
  advance(1000); TEST_ASSERT_TRUE(player_holding()); TEST_ASSERT_EQUAL(3, player_frame());
  advance(500); advance(200); TEST_ASSERT_EQUAL(2, player_frame());
}
int main() {
  UNITY_BEGIN();
  RUN_TEST(forward_cycle_is_never_reversed);
  RUN_TEST(walking_finishes_forward_even_on_quick_stop);
  RUN_TEST(tilt_keeps_swaying_and_gets_back_up);
  RUN_TEST(tired_pose_waits_before_getting_up);
  RUN_TEST(interrupted_forward_action_never_replays_backwards);
  return UNITY_END();
}
