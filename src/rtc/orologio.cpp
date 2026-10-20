#include "orologio.h"
#include <SensorPCF85063.hpp>
#include <Wire.h>

static SensorPCF85063 rtc;
static bool rtc_ok = false;

// __DATE__ e' tipo "Oct 20 2026"
static int build_month() {
  const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  for (int i = 0; i < 12; i++) {
    if (strncmp(__DATE__, months + i * 3, 3) == 0) {
      return i + 1;
    }
  }
  return 1;
}

void rtc_init() {
  // il bus I2C e' gia' acceso dall'IMU
  if (!rtc.begin(Wire)) {
    Serial.println("RTC PCF85063 non trovato!");
    return;
  }
  rtc_ok = true;

  // mai impostato o rimasto senza corrente: usa l'ora della compilazione
  RTC_DateTime now = rtc.getDateTime();
  if (!rtc.isClockIntegrityGuaranteed() || now.getYear() < 2026) {
    rtc.setDateTime(atoi(__DATE__ + 7), build_month(), atoi(__DATE__ + 4),
                    atoi(__TIME__), atoi(__TIME__ + 3), atoi(__TIME__ + 6));
    Serial.println("RTC impostato all'ora della compilazione");
  }

  now = rtc.getDateTime();
  Serial.printf("Ora: %02d:%02d\n", now.getHour(), now.getMinute());
}

int rtc_hour() {
  if (!rtc_ok) {
    return -1;
  }
  return rtc.getDateTime().getHour();
}
