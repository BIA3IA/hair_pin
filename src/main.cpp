#include "display/display.h"
#include "imu/imu.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  display_init();
  imu_init();

  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {
  display_test();
  imu_read_print();
  delay(1000);
}
