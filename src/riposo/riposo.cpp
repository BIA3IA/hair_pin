#include "riposo.h"
#include "../display/display.h"
#include "../imu/imu.h"
#include "../touch/tocchi_display.h"
#include <BLEDevice.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

// il deep sleep si sveglia solo da GPIO0-7, tutti occupati: il touch e' su GPIO11
const gpio_num_t BAT_EN = GPIO_NUM_15;

void riposo_dormi() {
  Serial.println("Buonanotte: tocca lo schermo per svegliarmi");
  delay(300); // lascia partire la risposta Bluetooth al comando

  BLEDevice::deinit(false);
  imu_sleep();
  for (int level = 120; level > 0; level -= 8) {
    display_set_brightness(level);
    delay(15);
  }
  display_set_brightness(0);
  gfx->displayOff();

  // a batteria BAT_EN basso spegnerebbe tutta la scheda
  gpio_hold_en(BAT_EN);
  gpio_hold_en((gpio_num_t)LCD_BL);

  detachInterrupt(TOUCH_IRQ);
  while (digitalRead(TOUCH_IRQ) == LOW) delay(10); // aspetta che il dito si stacchi
  gpio_wakeup_enable((gpio_num_t)TOUCH_IRQ, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();

  gpio_hold_dis(BAT_EN);
  gpio_hold_dis((gpio_num_t)LCD_BL);
  esp_restart();
}
