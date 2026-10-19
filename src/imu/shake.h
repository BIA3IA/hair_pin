#include <Arduino.h>
#pragma once

struct SensorShakeState {
  bool shaking;
  bool rotating;        // soglia bassa, serve alla camminata
  bool strong_rotation; // soglia alta, conta come scossa
};

float accel_magnitude(float ax, float ay, float az);
SensorShakeState is_shaking(float magnitude, float gyro_magnitude,
                            bool currently_shaking);
