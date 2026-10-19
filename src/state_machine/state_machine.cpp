#include "state_machine.h"

static State current_state = State::IDLE;

static const unsigned long SITTING_THRESHOLD_MS = 10000;
static const unsigned long SLEEPING_THRESHOLD_MS = 30000;
static unsigned long idle_since = 0;
static const unsigned long SLOWDOWN_MS = 1500; // quanto camminare dopo aver corso
static unsigned long last_running_time = 0;

void state_machine_update(SensorState sensor_state) {

    if (sensor_state.running) {

    current_state = State::RUNNING;
    last_running_time = millis();

  } else if (sensor_state.walking) {

    current_state = State::WALKING;

  } else if (sensor_state.upside_down) {

    current_state = State::UPSIDE_DOWN;

  } else if (sensor_state.shaking) {

    current_state = State::SHAKEN;

  } else if (sensor_state.tilted) {

    current_state = State::TILTED;

    } else if (last_running_time != 0 &&
             millis() - last_running_time < SLOWDOWN_MS) {
    current_state = State::WALKING; // stava correndo da poco: rallenta camminando
  } else {

    if (current_state != State::IDLE && current_state != State::SITTING &&
        current_state != State::SLEEPING) {
      current_state = State::IDLE;
      idle_since = millis();
    }

    unsigned long idle_duration = millis() - idle_since;
    if (idle_duration >= SLEEPING_THRESHOLD_MS) {
      current_state = State::SLEEPING;
    } else if (idle_duration >= SITTING_THRESHOLD_MS) {
      current_state = State::SITTING;
    }
  }
}

State state_machine_current() { return current_state; }