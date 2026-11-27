#include <Arduino.h>
#pragma once

void rtc_init();
int rtc_hour(); // -1 se l'orologio non risponde
void rtc_set(int year, int month, int day, int hour, int minute, int second);
