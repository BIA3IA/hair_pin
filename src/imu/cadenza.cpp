#include "cadenza.h"

bool is_walking(bool step_this_tick, bool is_rotating) {
  const unsigned long MIN_STEP_INTERVAL_MS = 250;
  const unsigned long MAX_STEP_INTERVAL_MS = 900;
  const float MAX_INTERVAL_RATIO = 1.5;
  const unsigned long WALKING_TIMEOUT_MS = 1200;

  static unsigned long last_step_time = 0;
  static unsigned long prev_interval = 0;
  static int consistent_steps = 0;
  static bool walking = false;

  unsigned long now = millis();

  // passo valido se c'è stato un passo in questo tick e non stiamo ruotando
  const bool count_this_step = step_this_tick && !is_rotating;

  if (count_this_step) {
    if (last_step_time != 0) {
      unsigned long interval = now - last_step_time;

      if (interval >= MIN_STEP_INTERVAL_MS &&
          interval <= MAX_STEP_INTERVAL_MS) {
        if (prev_interval != 0) {
          float ratio = (float)interval / (float)prev_interval;
          if (ratio < 1.0) {
            ratio = 1.0 / ratio;
          }

          if (ratio <= MAX_INTERVAL_RATIO) {
            consistent_steps++;
          } else {
            consistent_steps = 0;
          }
        }
        prev_interval = interval;
      } else {
        // intervallo troppo corto o troppo lungo per essere un passo credibile
        consistent_steps = 0;
        prev_interval = 0;
      }
    }
    last_step_time = now;

    if (consistent_steps >= 2) {
      walking = true;
    }
  }

  if (walking && (now - last_step_time) > WALKING_TIMEOUT_MS) {
    // troppo tempo senza un vero passo
    walking = false;
    consistent_steps = 0;
    prev_interval = 0;
  }

  return walking;
}
