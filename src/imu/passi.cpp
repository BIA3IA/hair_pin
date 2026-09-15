#include "passi.h"

bool step_detection(float magnitude) {
  const float STEP_THRESHOLD = 0.15;
  const unsigned long REFRACTORY_MS = 250;

  static bool above_threshold = false;
  static unsigned long last_step_time = 0;

  unsigned long now = millis();

  if (!above_threshold && magnitude > STEP_THRESHOLD &&
      (now - last_step_time) > REFRACTORY_MS) {
    above_threshold = true;
    last_step_time = now;
    return true;
  }

  if (magnitude < STEP_THRESHOLD) {
    above_threshold = false;
  }

  return false;
}
