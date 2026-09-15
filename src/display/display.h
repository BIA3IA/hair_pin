#include <Arduino.h>

#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

#define GFX_BL LCD_BL

void display_init();
void display_test();