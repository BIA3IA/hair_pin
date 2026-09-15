#include "display.h"
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, LCD_RST, 0 /* rotation */, true /* IPS */, 240 /* width */,
    280 /* height */, 0 /* col offset 1 */, 20 /* row offset 1 */,
    0 /* col offset 2 */, 20 /* row offset 2 */);

void display_init() {
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
}

void display_test() {
  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_RED);
  gfx->println("Hello World!");
}
