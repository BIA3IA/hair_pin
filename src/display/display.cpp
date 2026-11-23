#include "display.h"
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN,
                                             GFX_NOT_DEFINED, FSPI, false);
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, LCD_RST, 1 /* rotation */, true /* IPS */, 240 /* width */,
    280 /* height */, 0 /* col offset 1 */, 20 /* row offset 1 */,
    0 /* col offset 2 */, 20 /* row offset 2 */);

void display_init() {
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);
  display_set_brightness(255);
}

void display_set_brightness(uint8_t level) {
  static int current = -1;
  if (level == current) {
    return;
  }
  current = level;
  analogWrite(LCD_BL, level);
}
