#include "tocchi_display.h"
#include "../imu/imu.h"
#include <TouchDrv.hpp>
#include <Wire.h>

static TouchDrvCSTXXX touch;
static volatile bool irq = false;
static unsigned long last_irq_time = 0;

void touch_init() {
  touch.setTouchDrvModel(TouchDrv_CST8XX);
  if (!touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("Touch CST816 non trovato!");
    return;
  }
  attachInterrupt(TOUCH_IRQ, []() { irq = true; }, FALLING);
}

bool touch_tapped() {
  if (!irq) {
    return false;
  }
  irq = false;

  // leggere il punto libera il chip per il tocco successivo
  if (touch.getTouchPoints().getPointCount() == 0) {
    return false;
  }

  unsigned long now = millis();
  bool new_tap = now - last_irq_time > 300;
  last_irq_time = now;
  return new_tap;
}
