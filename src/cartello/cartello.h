#include <Arduino.h>
#pragma once
#include "../umore/umore.h"

enum class Icon : uint8_t { NONE, HEART, STEPS, MOON, STAR };

struct SignContent {
  const char *text; // "" = solo l'icona
  Icon icon;
};

SignContent sign_pick_phrase(int hour, Mood mood, unsigned long steps);
void sign_draw(const SignContent &c, int x, int y, int w, int h);
void sign_check_phrases(int w, int h); // stampa quelle che non ci stanno
