#include <Arduino.h>
#pragma once

// umore (0-100) e passi del giorno, restano anche da spenta
enum class Mood { TRISTE, GIU, NORMALE, FELICE };

void umore_init(long today);
void umore_update(long today); // basta una volta al minuto
void umore_add(int delta);
void umore_attention(); // l'hai toccata o state camminando insieme
void umore_add_steps(int steps);
int umore_value();
Mood umore_level();
unsigned long umore_steps_today();
