#include <Arduino.h>

#define SCREEN_WIDTH 280
#define SCREEN_HEIGHT 240

void sprite_init();
void sprite_draw_frame(const char *path, int frameIndex, float scale);
int sprite_get_frame_count(const char *path);
void sprite_get_draw_rect(const char *path, float scale, int *x, int *y,
                          int *dw, int *dh);
void rect_union(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh,
                int *ux, int *uy, int *uw, int *uh);
void sprite_draw_frame_buffered(const char *path, int frameIndex, float scale,
                                int prev_x, int prev_y, int prev_dw,
                                int prev_dh, int *out_x, int *out_y,
                                int *out_dw, int *out_dh);
