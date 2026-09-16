#include "shake.h"

float accel_magnitude(float ax, float ay, float az) {
  return sqrtf(ax * ax + ay * ay + az * az);
}

SensorShakeState is_shaking(float magnitude, float gyro_magnitude,
                            bool currently_shaking) {
  bool shaking = false;

  const float ENTRY_THRESHOLD = 0.4;
  const float EXIT_THRESHOLD = 0.2;

  const int ROTATION_PEAK_HOLD_MS = 500;
  const float ROTATION_THRESHOLD = 90.0;

  unsigned long now = millis();

  static float gyro_peak = 0;
  static unsigned long gyro_peak_time = 0;

  const float deviation = fabsf(magnitude - 1.0f);

  if (currently_shaking) {
    shaking = deviation > EXIT_THRESHOLD;
  } else {
    shaking = deviation > ENTRY_THRESHOLD;
  }

  if (gyro_magnitude > gyro_peak ||
      (now - gyro_peak_time) > ROTATION_PEAK_HOLD_MS) {
    gyro_peak = gyro_magnitude;
    gyro_peak_time = now;
  }

  const bool is_rotating = gyro_peak > ROTATION_THRESHOLD;

  SensorShakeState state = {shaking, is_rotating};
  return state;
}