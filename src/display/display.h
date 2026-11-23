#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

extern Arduino_GFX *gfx;

void display_init();
void display_set_brightness(uint8_t level); // 0 spento, 255 massimo
