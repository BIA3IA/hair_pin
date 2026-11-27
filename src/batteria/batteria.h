#pragma once
#include <stdint.h>

void batteria_init();
void batteria_update();
// Stima da tensione, 255 se la misura non e' utilizzabile.
uint8_t batteria_percentuale();
