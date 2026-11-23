#pragma once
#include <Arduino.h>

// muove le forme senza toccare la zona di Bianca; true se una le passa dietro
bool background_update(int x, int y, int w, int h);
// riempie il buffer con lo sfondo di quella zona
void background_compose(uint16_t *buffer, int x, int y, int w, int h);
