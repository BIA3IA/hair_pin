#pragma once
#include <Arduino.h>

#define SCREEN_WIDTH 280
#define SCREEN_HEIGHT 240
#define SPRITE_HEADER_SIZE 14

constexpr float SPRITE_SCALE = 1.7f;

struct SpriteHeader {
  uint16_t w, h;
  uint16_t frame_count;
  uint16_t canvas_w, canvas_h;
  uint16_t offset_x, offset_y; // dove inizia il ritaglio nella tela
};

void sprite_init();
int sprite_get_frame_count(const char *path);
void sprite_canvas_origin(const char *path, int *x, int *y);
// da chiamare dopo aver disegnato sopra la sprite (es. il testo del cartello)
void sprite_invalidate();
// disegna il frame ridisegnando anche la zona lasciata libera dal frame prima
void sprite_draw_frame_buffered(const char *path, int frame, int prev_x,
                                int prev_y, int prev_w, int prev_h, int *out_x,
                                int *out_y, int *out_w, int *out_h);
