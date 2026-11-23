#pragma once
#include "../state_machine/state_machine.h"
#include "../touch/tocchi_display.h"

// Decide cosa fa Bianca: animazione dello stato, variazioni, reazioni, cartello.
void comportamento_init(int hour);
void comportamento_update(State target, TouchEvent touch, bool jolt, int hour,
                          bool background_moved);
