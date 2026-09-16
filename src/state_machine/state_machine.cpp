#include "state_machine.h"

static State current_state = State::IDLE;

static const unsigned long SITTING_THRESHOLD_MS = 10000;
static const unsigned long SLEEPING_THRESHOLD_MS = 30000;
static unsigned long idle_since = 0;

void state_machine_update(SensorState sensor_state) {

  if (sensor_state.walking) {

    current_state = State::WALKING;

  } else if (sensor_state.tilted) {

    current_state = State::TILTED;

  } else if (sensor_state.upside_down) {

    current_state = State::UPSIDE_DOWN;

  } else if (sensor_state.shaking) {

    current_state = State::SHAKEN;

  } else {

    if (current_state != State::IDLE) {
      idle_since = millis();
    }
    current_state = State::IDLE;
  }
}

State state_machine_current() { return current_state; }