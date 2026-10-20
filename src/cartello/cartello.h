#include <Arduino.h>
#pragma once

const char *sign_pick_phrase(int hour);
void sign_draw_text(const char *text, int x, int y, int w, int h);
void sign_check_phrases(int w, int h); // stampa quelle che non ci stanno
