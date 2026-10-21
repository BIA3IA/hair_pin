#pragma once
#include <Arduino.h>

// Aggiorna lo sfondo senza toccare il rettangolo del personaggio.
void background_update(int x, int y, int w, int h);

// Compone lo sfondo dietro i pixel trasparenti dello sprite.
void background_compose(uint16_t *buffer, int x, int y, int w, int h);
