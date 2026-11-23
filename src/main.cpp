#include "ble/collegamento.h"
#include "comportamento/comportamento.h"
#include "display/background.h"
#include "display/display.h"
#include "imu/imu.h"
#include "imu/registro.h"
#include "player/player.h"
#include "rtc/orologio.h"
#include "sprites/sprite.h"
#include "state_machine/state_machine.h"
#include "touch/tocchi_display.h"
#include "umore/umore.h"
#include <Arduino.h>

const unsigned long IMU_INTERVAL_MS = 20; // 50 Hz, come i contapassi
const unsigned long CLOCK_INTERVAL_MS = 60000;

static int hour_now = -1;

static void handle_bluetooth() {
  NotifyPacket notify;
  if (!collegamento_connected()) comportamento_clear_phrases();
  while (collegamento_take_notify(&notify)) {
    if (notify.flags == 0) comportamento_notify(static_cast<Icon>(notify.icon), notify.text);
    else if (collegamento_connected()) comportamento_command(notify.flags, notify.text);
  }
  TimePacket t;
  if (collegamento_take_time(&t)) {
    rtc_set(t.year, t.month, t.day, t.hour, t.minute, t.second);
    hour_now = rtc_hour();
    umore_update(rtc_date_key());
    Serial.printf("Ora dal telefono: %02d:%02d\n", t.hour, t.minute);
  }
  uint32_t passkey;
  if (collegamento_take_passkey(&passkey)) {
    comportamento_show_passkey(passkey);
  }
  if (collegamento_take_paired()) {
    comportamento_pairing_done();
    Serial.println("Telefono abbinato");
  }
  collegamento_update({static_cast<uint8_t>(umore_value()), umore_steps_today(),
                       comportamento_sleeping(), BATTERY_UNKNOWN});
}

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0); // senza monitor aperto ogni print aspetterebbe
  display_init();
  imu_init();
  touch_init();
  rtc_init();
  hour_now = rtc_hour();
  umore_init(rtc_date_key());
  sprite_init();
  registro_init();
  collegamento_init();
  comportamento_init(hour_now);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {
  static unsigned long last_imu = 0, last_clock = 0;
  unsigned long now = millis();

  if (now - last_imu >= IMU_INTERVAL_MS) {
    last_imu = now;
    state_machine_update(imu_update());
  }
  if (now - last_clock >= CLOCK_INTERVAL_MS) {
    last_clock = now;
    hour_now = rtc_hour();
    umore_update(rtc_date_key());
  }
  umore_add_steps(imu_take_steps());
  registro_poll_serial();
  handle_bluetooth();

  int x, y, w, h;
  player_area(&x, &y, &w, &h);
  bool background_moved = background_update(x, y, w, h);
  comportamento_update(state_machine_current(), touch_update(), imu_take_jolt(),
                       hour_now, background_moved);
  delay(1);
}
