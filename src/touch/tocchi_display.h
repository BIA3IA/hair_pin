#include <Arduino.h>
#pragma once

#define TOUCH_IRQ 11

enum class TouchEvent { NONE, TAP, LONG_PRESS };

void touch_init();
TouchEvent touch_update(); // da chiamare a ogni giro del loop
