#include "display/display.h"
#include "imu/imu.h"
#include "sprites/sprite.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  display_init();
  imu_init();
  sprite_init();
  sprite_draw_frame("/Idle/idle_1.bin", 0, 1.7);

  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() { delay(1000); }
