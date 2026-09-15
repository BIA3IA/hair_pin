#include <Arduino.h>
#pragma once

float accel_magnitude(float ax, float ay, float az);
bool is_shaking(float magnitude, bool currently_shaking);
