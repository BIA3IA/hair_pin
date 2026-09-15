#include <Arduino.h>

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

void state_machine_update(float angle, float magnitude, bool step_detected);
State state_machine_current();