#include <Arduino.h>
#pragma once

bool is_walking(bool step_this_tick, bool is_rotating);
bool is_running();
int walking_take_steps(); // passi nuovi dall'ultima chiamata
