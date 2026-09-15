#include <Arduino.h>
#pragma once

float tilt_angle(float ax, float ay, float az);
bool is_tilted(float angle, bool currently_tilted);
bool is_upside_down(float angle, bool currently_upside_down);
