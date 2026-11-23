#include <Arduino.h>
#pragma once

struct SensorShakeState {
  bool shaking;      // scossa forte che dura
  bool rotating;     // testa che si muove: i passi non contano
  bool head_shaking; // "no" con la testa: rotazione avanti e indietro
};

SensorShakeState is_shaking(float linear_accel, float gyro_magnitude,
                            float yaw_rate, float dt, bool currently_shaking);
