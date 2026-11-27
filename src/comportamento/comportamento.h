#pragma once
#include "../state_machine/state_machine.h"
#include "../cartello/cartello.h"
#include "../touch/tocchi_display.h"

// Decide cosa fa Bianca: animazione dello stato, variazioni, reazioni, cartello.
void comportamento_init(int hour);
void comportamento_update(State target, TouchEvent touch, bool jolt, int hour,
                          bool background_moved);
void comportamento_notify(Icon icon, const char *text);
void comportamento_show_passkey(uint32_t code);
void comportamento_pairing_done();
bool comportamento_sleeping();
void comportamento_set_brightness(uint8_t level); // salvata in memoria
uint8_t comportamento_brightness();

void comportamento_command(uint8_t action, const char *text);
void comportamento_clear_phrases();
