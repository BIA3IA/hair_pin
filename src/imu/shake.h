#include <Arduino.h>
#pragma once

struct SensorShakeState {
  bool shaking;
  bool rotating;
};

float accel_magnitude(float ax, float ay, float az);
SensorShakeState is_shaking(float magnitude, float gyro_magnitude,
                            bool currently_shaking);
