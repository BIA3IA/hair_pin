#include "umore.h"
#include <Preferences.h>

namespace {
Preferences prefs;
int mood = 60;
unsigned long steps = 0;
long steps_day = -1;
int steps_for_bonus = 0;
bool dirty = false;
unsigned long last_attention = 0;
unsigned long last_tick = 0;
unsigned long last_save = 0;

const int NEUTRAL = 60;
const int HAPPY_REST = 70; // da felice torna piano verso qui
const unsigned long TICK_MS = 20UL * 60 * 1000;
const unsigned long IGNORED_MS = 2UL * 60 * 60 * 1000;
const unsigned long SAVE_MS = 5UL * 60 * 1000; // la flash si consuma: non a ogni modifica
const int STEPS_PER_POINT = 150;

void new_day(long today) {
  if (today < 0 || today == steps_day) {
    return;
  }
  steps_day = today;
  steps = 0;
  dirty = true;
}

void save() {
  prefs.putInt("umore", mood);
  prefs.putULong("passi", steps);
  prefs.putLong("giorno", steps_day);
  dirty = false;
  last_save = millis();
}
} // namespace

void umore_init(long today) {
  prefs.begin("bianca", false);
  mood = constrain(prefs.getInt("umore", NEUTRAL), 0, 100);
  steps = prefs.getULong("passi", 0);
  steps_day = prefs.getLong("giorno", -1);
  new_day(today);
  last_attention = last_tick = last_save = millis();
  Serial.printf("Umore %d, passi oggi %lu\n", mood, steps);
}

void umore_update(long today) {
  unsigned long now = millis();
  new_day(today);

  if (now - last_tick >= TICK_MS) {
    last_tick = now;
    if (now - last_attention >= IGNORED_MS) {
      umore_add(-2); // la ignori da ore
    } else if (mood > HAPPY_REST) {
      umore_add(-1);
    } else if (mood < NEUTRAL) {
      umore_add(1); // ti ha vista di recente: si riprende
    }
  }

  if (dirty && now - last_save >= SAVE_MS) {
    save();
  }
}

void umore_add(int delta) {
  int next = constrain(mood + delta, 0, 100);
  if (next != mood) {
    mood = next;
    dirty = true;
    Serial.printf("Umore %d\n", mood);
  }
}

void umore_attention() { last_attention = millis(); }

void umore_add_steps(int n) {
  if (n <= 0) {
    return;
  }
  steps += n;
  dirty = true;
  umore_attention();
  steps_for_bonus += n;
  while (steps_for_bonus >= STEPS_PER_POINT) {
    steps_for_bonus -= STEPS_PER_POINT;
    umore_add(1); // le piace andare a spasso
  }
}

int umore_value() { return mood; }

Mood umore_level() {
  if (mood < 25)
    return Mood::TRISTE;
  if (mood < 45)
    return Mood::GIU;
  if (mood < 75)
    return Mood::NORMALE;
  return Mood::FELICE;
}

unsigned long umore_steps_today() { return steps; }
