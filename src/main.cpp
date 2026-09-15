#include "display/display.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  display_init();

  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {
  display_test();
  delay(1000);
}
