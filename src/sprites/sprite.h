#include <Arduino.h>

#define SCREEN_WIDTH 280
#define SCREEN_HEIGHT 240

#define SPRITE_HEADER_SIZE 14

struct SpriteHeader {
  uint16_t w, h;
  uint16_t frame_count;
  uint16_t canvas_w, canvas_h;
  uint16_t offset_x, offset_y; // dove inizia il ritaglio nella tela
};

void sprite_init();
void sprite_draw_frame(const char *path, int frameIndex, float scale);
int sprite_get_frame_count(const char *path);
void sprite_get_draw_rect(const char *path, float scale, int *x, int *y,
                          int *dw, int *dh);
void sprite_canvas_origin(const char *path, float scale, int *x, int *y);
void rect_union(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh,
                int *ux, int *uy, int *uw, int *uh);
void sprite_draw_frame_buffered(const char *path, int frameIndex, float scale,
                                int prev_x, int prev_y, int prev_dw,
                                int prev_dh, int *out_x, int *out_y,
                                int *out_dw, int *out_dh);
