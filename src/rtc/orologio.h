#include <Arduino.h>
#pragma once

void rtc_init();
int rtc_hour(); // -1 se l'orologio non risponde
long rtc_date_key(); // es. 20261123, -1 se non disponibile
