#include "display/display.h"
#include "imu/imu.h"
#include "sprites/sprite.h"
#include "state_machine/state_machine.h"
#include <Arduino.h>

const char *animation_path_for_state(State state) {
  switch (state) {
  case State::WALKING:
    return "/Idle/walking.bin";
  case State::TILTED:
    return "/Idle/lose_balance.bin";
  case State::SHAKEN:
    return "/Idle/shake.bin";
  case State::UPSIDE_DOWN:
    return "/Idle/upsidedown.bin";
  case State::SITTING:
    return "/sitting_down/Idle.bin";
  case State::SLEEPING:
    return "/Idle/sleepy.bin";
  case State::RUNNING:
  return "/Idle/running.bin";
  default:
    return "/Idle/idle_1.bin";
  }
}

// quanti frame finali ripetere in loop
// 0 = ripeti tutto
int hold_frames_for_state(State state) {
  switch (state) {
  case State::TILTED:
    return 5;
  case State::SHAKEN:
    return 3;
  case State::UPSIDE_DOWN:
    return 3;
  default:
    return 0;
  }
}

bool reverse_on_exit(State state) {
  switch (state) {
  case State::TILTED:
  case State::UPSIDE_DOWN:
  case State::SHAKEN:
    return true;
  default:
    return false;
  }
}

static int current_frame = 0;
static int frame_count = 1;
static unsigned long last_frame_time = 0;
const unsigned long FRAME_INTERVAL_MS = 100;
static int prev_x = 0, prev_y = 0, prev_dw = 0, prev_dh = 0;

const unsigned long HOLD_FRAME_INTERVAL_MS = 180; // più lento su finale
static bool holding = false; // true = avanti-indietro
static int frame_dir = 1;    // +1 avanti, -1 indietro

static State shown_state = State::IDLE;
static bool reversing = false;

void draw_animation_frame(const char *path) {
  sprite_draw_frame_buffered(path, current_frame, 1.7, prev_x, prev_y, prev_dw,
                             prev_dh, &prev_x, &prev_y, &prev_dw, &prev_dh);
}

void advance_frame(int hold) {
  if (!holding) {
    current_frame++;
    if (current_frame < frame_count) {
      return; // primo giuro
    }
    if (hold <= 0) {
      current_frame = 0;
      return;
    }

    holding = true;
    frame_dir = -1;
    current_frame = frame_count - 1;
  }

  int first = max(0, frame_count - hold); 
  int last = frame_count - 1;             

  current_frame += frame_dir;
  if (current_frame <= first) {
    current_frame = first;
    frame_dir = 1; 
  }
  if (current_frame >= last) {
    current_frame = last;
    frame_dir = -1; 
  }
}

void start_animation(State state) {
  shown_state = state;
  const char *path = animation_path_for_state(state);
  frame_count = sprite_get_frame_count(path);
  current_frame = 0;
  holding = false;
  frame_dir = 1;
  reversing = false;
  last_frame_time = millis();
  draw_animation_frame(path);
  Serial.println("Current state: " + String(static_cast<int>(state)));
}

void setup() {
  Serial.begin(115200);
  display_init();
  imu_init();
  sprite_init();
  sprite_draw_frame("/Idle/idle_1.bin", 0, 1.7);
  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {
  state_machine_update(imu_update());
  State target = state_machine_current();

  if (target != shown_state) {
    if (reverse_on_exit(shown_state) && current_frame > 0) {
      reversing = true;
      holding = false;
    } else {
      start_animation(target);
    }
  } else if (reversing) {
    reversing = false;
    frame_dir = 1;
  }

  unsigned long interval = holding ? HOLD_FRAME_INTERVAL_MS : FRAME_INTERVAL_MS;
  if (millis() - last_frame_time >= interval) {
    if (reversing) {
      current_frame--;
    } else {
      advance_frame(hold_frames_for_state(shown_state));
    }
    last_frame_time = millis();
    draw_animation_frame(animation_path_for_state(shown_state));
  }

  delay(20);
}
