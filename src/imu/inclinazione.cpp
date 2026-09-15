#include "inclinazione.h"

float tilt_angle(float ax, float ay, float az) {
  return atan2f(sqrtf(ax * ax + az * az), ay) * RAD_TO_DEG;
}
