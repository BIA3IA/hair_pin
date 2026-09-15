#include "display/display.h"
#include "imu/imu.h"
#include "sprites/sprite.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  display_init();
  imu_init();
  sprite_init();

  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {

  sprite_draw_frame("/Idle/idle_1.bin", 0, 40, 40);

  delay(1000);
}
