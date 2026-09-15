#include "shake.h"

float accel_magnitude(float ax, float ay, float az) {
  return sqrtf(ax * ax + ay * ay + az * az);
}

bool is_shaking(float magnitude, bool currently_shaking) {
  const float ENTRY_THRESHOLD = 0.5;
  const float EXIT_THRESHOLD = 0.3;

  const float deviation = fabsf(magnitude - 1.0f);

  if (currently_shaking) {
    return deviation > EXIT_THRESHOLD;
  } else {
    return deviation > ENTRY_THRESHOLD;
  }
}