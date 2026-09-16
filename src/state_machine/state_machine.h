#include "imu/imu.h"
#include <Arduino.h>
#pragma once

enum class State {
  IDLE,
  WALKING,
  RUNNING,
  TILTED,
  SHAKEN,
  UPSIDE_DOWN,
  SITTING,
  SLEEPING,
  ANNOYED,
  NOTIFICATION
};

void state_machine_update(SensorState sensor_state);
State state_machine_current();