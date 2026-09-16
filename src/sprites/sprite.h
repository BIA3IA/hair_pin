#include <Arduino.h>

#define SCREEN_WIDTH 280
#define SCREEN_HEIGHT 240

void sprite_init();
void sprite_draw_frame(const char *path, int frameIndex, float scale);
int sprite_get_frame_count(const char *path);
void sprite_get_draw_rect(const char *path, float scale, int *x, int *y,
                          int *dw, int *dh);
