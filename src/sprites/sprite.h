#include <Arduino.h>

#define SCREEN_WIDTH 280
#define SCREEN_HEIGHT 240

void sprite_init();
void sprite_draw_frame(const char *path, int frameIndex, float scale);