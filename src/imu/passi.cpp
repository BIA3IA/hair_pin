#include "passi.h"

// vertical = accelerazione lungo la gravita' (g), senza la gravita' stessa
bool step_detection(float vertical) {
  const float STEP_THRESHOLD = 0.12; // in testa un passo da' ~0.1-0.4 g
  const float RELEASE_THRESHOLD = 0.04;
  const unsigned long REFRACTORY_MS = 280;

  static bool above_threshold = false;
  static unsigned long last_step_time = 0;

  unsigned long now = millis();

  if (!above_threshold && vertical > STEP_THRESHOLD &&
      (now - last_step_time) > REFRACTORY_MS) {
    above_threshold = true;
    last_step_time = now;
    return true;
  }

  if (vertical < RELEASE_THRESHOLD) {
    above_threshold = false;
  }

  return false;
}
