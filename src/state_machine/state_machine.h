#pragma once
#include "../imu/imu.h"

enum class State { IDLE, WALKING, RUNNING, TILTED, SHAKEN, UPSIDE_DOWN, ANNOYED };

void state_machine_update(SensorState sensor_state);
State state_machine_current();
