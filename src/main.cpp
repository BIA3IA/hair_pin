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

enum class ExitMode {
  CUT,          // cambia subito
  REVERSE,      // riavvolge fino al frame 0 poi cambia
  FINISH_CYCLE, // finisce il ciclo in corso poi cambia
};

ExitMode exit_mode_for_state(State state) {
  switch (state) {
  case State::WALKING:
  case State::RUNNING:
    return ExitMode::FINISH_CYCLE;
  case State::IDLE:
    return ExitMode::CUT;
  default:
    return ExitMode::REVERSE;
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

const char *YAWN_PATH = "/Idle/sleepy.bin";
const unsigned long YAWN_MIN_MS = 30000; // sbadiglia dopo almeno 30 s di idle...
const unsigned long YAWN_MAX_MS = 60000; // ...e al massimo dopo 60 s

static bool yawning = false;               // true = sta sbadigliando
static unsigned long yawn_timer_start = 0; // da quando contiamo
static unsigned long yawn_delay = 0;       // quanto aspettare (casuale)


void draw_animation_frame(const char *path) {
  sprite_draw_frame_buffered(path, current_frame, 1.7, prev_x, prev_y, prev_dw,
                             prev_dh, &prev_x, &prev_y, &prev_dw, &prev_dh);
}

void schedule_next_yawn() {
  yawn_timer_start = millis();
  yawn_delay = random(YAWN_MIN_MS, YAWN_MAX_MS);
}

void start_yawn() {
  yawning = true;
  frame_count = sprite_get_frame_count(YAWN_PATH);
  current_frame = 0;
  last_frame_time = millis();
  draw_animation_frame(YAWN_PATH);
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
  yawning = false;
  if (state == State::IDLE) {
    schedule_next_yawn(); // ogni volta che torna in idle riparte il conto
  }

}

void setup() {
  Serial.begin(115200);
  display_init();
  imu_init();
  sprite_init();
  start_animation(State::IDLE);
  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {
  state_machine_update(imu_update());
  State target = state_machine_current();

  if (target != shown_state) {
    ExitMode mode = exit_mode_for_state(shown_state);
    if (mode == ExitMode::CUT || current_frame == 0) {
      start_animation(target);
    } else if (mode == ExitMode::REVERSE) {
      reversing = true;
      holding = false;
    }
  } else if (reversing) {
    reversing = false;
    frame_dir = 1;
  } else if (shown_state == State::IDLE && !yawning &&
             millis() - yawn_timer_start >= yawn_delay) {
    start_yawn();
  }


  unsigned long interval = holding ? HOLD_FRAME_INTERVAL_MS : FRAME_INTERVAL_MS;

  if (millis() - last_frame_time >= interval) {
    if (yawning) {
      current_frame++;
      if (current_frame >= frame_count) {
        start_animation(State::IDLE); // sbadiglio finito: torna in idle
        return;
      }
      last_frame_time = millis();
      draw_animation_frame(YAWN_PATH);
    } else {
      if (reversing) {
        current_frame--;
      } else {
        advance_frame(hold_frames_for_state(shown_state));
      }
      last_frame_time = millis();
      draw_animation_frame(animation_path_for_state(shown_state));
    }
  }

  delay(20);
}
