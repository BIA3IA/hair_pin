#include "tocchi_display.h"
#include "../imu/imu.h"
#include <TouchDrv.hpp>
#include <Wire.h>

static TouchDrvCSTXXX touch;
static volatile bool irq = false;
static bool ready = false;

static bool pressed = false;
static bool long_sent = false;
static unsigned long press_start = 0;
static unsigned long last_poll = 0;
static unsigned long last_release = 0;

const unsigned long LONG_PRESS_MS = 700;
const unsigned long POLL_MS = 40;
const unsigned long DEBOUNCE_MS = 150;

void touch_init() {
  touch.setTouchDrvModel(TouchDrv_CST8XX);
  if (!touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("Touch CST816 non trovato!");
    return;
  }
  ready = true;
  attachInterrupt(TOUCH_IRQ, []() { irq = true; }, FALLING);
}

// il dito e' ancora sullo schermo? (leggere il punto libera anche il chip)
static bool finger_down() { return touch.getTouchPoints().getPointCount() > 0; }

TouchEvent touch_update() {
  if (!ready) {
    return TouchEvent::NONE;
  }
  unsigned long now = millis();

  if (!pressed) {
    if (!irq) {
      return TouchEvent::NONE;
    }
    irq = false;
    if (!finger_down() || now - last_release < DEBOUNCE_MS) {
      return TouchEvent::NONE;
    }
    pressed = true;
    long_sent = false;
    press_start = now;
    last_poll = now;
    return TouchEvent::NONE;
  }

  // mentre e' premuto controlla ogni tanto se il dito c'e' ancora
  if (now - last_poll < POLL_MS) {
    return TouchEvent::NONE;
  }
  last_poll = now;
  irq = false;

  if (finger_down()) {
    if (!long_sent && now - press_start >= LONG_PRESS_MS) {
      long_sent = true;
      return TouchEvent::LONG_PRESS;
    }
    return TouchEvent::NONE;
  }

  pressed = false;
  last_release = now;
  return long_sent ? TouchEvent::NONE : TouchEvent::TAP;
}
