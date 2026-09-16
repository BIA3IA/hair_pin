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
  default:
    return "/Idle/idle_1.bin";
  }
}

static int current_frame = 0;
static int frame_count = 1;
static unsigned long last_frame_time = 0;
const unsigned long FRAME_INTERVAL_MS = 50;

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

  State previous_state = state_machine_current();

  state_machine_update(imu_update());

  State current_state = state_machine_current();

  const char *path = animation_path_for_state(current_state);

  if (current_state != previous_state) {
    frame_count = sprite_get_frame_count(path);
    current_frame = 0;
    last_frame_time = millis();
    gfx->fillScreen(RGB565_BLACK);
    sprite_draw_frame(path, current_frame, 1.7);
    Serial.println("Current state: " + String(static_cast<int>(current_state)));
  } else if (millis() - last_frame_time >= FRAME_INTERVAL_MS) {
    current_frame = (current_frame + 1) % frame_count;
    last_frame_time = millis();
    gfx->fillScreen(RGB565_BLACK);
    sprite_draw_frame(path, current_frame, 1.7);
  }

  delay(20);
}