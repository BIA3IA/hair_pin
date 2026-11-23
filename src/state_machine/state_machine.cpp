#include "state_machine.h"

static State current_state = State::IDLE;

static const unsigned long SLOWDOWN_MS = 1500; // dopo la corsa cammina un po'
static unsigned long last_running_time = 0;

void state_machine_update(SensorState sensor) {
  if (sensor.running) {
    current_state = State::RUNNING;
    last_running_time = millis();
  } else if (sensor.walking) {
    current_state = State::WALKING;
  } else if (sensor.upside_down) {
    current_state = State::UPSIDE_DOWN;
  } else if (sensor.shaking) {
    current_state = State::SHAKEN;
  } else if (sensor.tilted) {
    current_state = State::TILTED;
  } else if (last_running_time != 0 && millis() - last_running_time < SLOWDOWN_MS) {
    current_state = State::WALKING;
  } else {
    current_state = State::IDLE;
  }
}

State state_machine_current() { return current_state; }
