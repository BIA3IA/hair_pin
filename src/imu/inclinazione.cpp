#include "inclinazione.h"

float tilt_angle(float ax, float ay, float az) {
  return atan2f(sqrtf(ax * ax + az * az), ay) * RAD_TO_DEG;
}

bool is_tilted(float angle, bool currently_tilted) {
  const float ENTRY_THRESHOLD = 40.0;
  const float EXIT_THRESHOLD = 30.0;
  if (currently_tilted) {
    return angle > EXIT_THRESHOLD;
  } else {
    return angle > ENTRY_THRESHOLD;
  }
}

bool is_upside_down(float angle, bool currently_upside_down) {
  const float UPSIDE_DOWN_THRESHOLD = 120.0;
  const float UPSIDE_UP_THRESHOLD = 140.0;
  if (currently_upside_down) {
    return angle > UPSIDE_DOWN_THRESHOLD;
  } else {
    return angle > UPSIDE_UP_THRESHOLD;
  }
}