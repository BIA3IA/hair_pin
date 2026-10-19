#include "sprite.h"
#include "../display/display.h"
#include <LittleFS.h>

static SpriteHeader read_header(File &f) {
  uint8_t b[SPRITE_HEADER_SIZE];
  f.read(b, SPRITE_HEADER_SIZE);
  SpriteHeader hd;
  hd.w = b[0] | (b[1] << 8); // little-endian: byte basso prima
  hd.h = b[2] | (b[3] << 8);
  hd.frame_count = b[4] | (b[5] << 8);
  hd.canvas_w = b[6] | (b[7] << 8);
  hd.canvas_h = b[8] | (b[9] << 8);
  hd.offset_x = b[10] | (b[11] << 8);
  hd.offset_y = b[12] | (b[13] << 8);
  return hd;
}

// centra la tela originale, non il ritaglio
static void screen_rect(const SpriteHeader &hd, float scale, int *x, int *y,
                        int *dw, int *dh) {
  *dw = (int)(hd.w * scale);
  *dh = (int)(hd.h * scale);
  *x = (SCREEN_WIDTH - (int)(hd.canvas_w * scale)) / 2 +
       (int)(hd.offset_x * scale);
  *y = (SCREEN_HEIGHT - (int)(hd.canvas_h * scale)) / 2 +
       (int)(hd.offset_y * scale);
}

void sprite_init() {
  if (!LittleFS.begin()) {
    Serial.println("Errore montaggio LittleFS!");
    return;
  }

  File f = LittleFS.open("/Idle/idle_1.bin", "r");
  if (!f) {
    Serial.println("File non trovato!");
    return;
  }

  SpriteHeader hd = read_header(f);
  f.close();

  Serial.printf("width=%d height=%d frames=%d canvas=%dx%d offset=%d,%d\n",
                hd.w, hd.h, hd.frame_count, hd.canvas_w, hd.canvas_h,
                hd.offset_x, hd.offset_y);
}

void sprite_draw_frame(const char *path, int frameIndex, float scale) {
  File f = LittleFS.open(path, "r");
  if (!f) {
    Serial.printf("Impossibile aprire: %s\n", path);
    return;
  }

  SpriteHeader hd = read_header(f);
  uint16_t w = hd.w;
  uint16_t h = hd.h;

  int maskBytes = (w * h + 7) / 8;
  int pixelBytes = w * h * 2;
  int frameBytes = maskBytes + pixelBytes;

  f.seek(SPRITE_HEADER_SIZE + frameIndex * frameBytes);

  uint8_t *mask = new uint8_t[maskBytes];
  uint8_t *pixels = new uint8_t[pixelBytes];
  f.read(mask, maskBytes);
  f.read(pixels, pixelBytes);
  f.close();

  int x, y, dw, dh;
  screen_rect(hd, scale, &x, &y, &dw, &dh);

  gfx->startWrite();
  for (int dy = 0; dy < dh; dy++) {
    int sy = (int)(dy / scale); // riga del pixel sorgente piu' vicino
    for (int dx = 0; dx < dw; dx++) {
      int sx = (int)(dx / scale); // colonna del pixel sorgente piu' vicino
      int i = sy * w + sx;
      bool opaque = (mask[i / 8] >> (i % 8)) & 1;
      if (opaque) {
        uint16_t color = pixels[i * 2] | (pixels[i * 2 + 1] << 8);
        gfx->writePixel(x + dx, y + dy, color);
      }
    }
  }
  gfx->endWrite();

  delete[] mask;
  delete[] pixels;
}

int sprite_get_frame_count(const char *path) {
  File f = LittleFS.open(path, "r");
  if (!f)
    return 1;

  SpriteHeader hd = read_header(f);
  f.close();

  return hd.frame_count;
}

void sprite_get_draw_rect(const char *path, float scale, int *x, int *y,
                          int *dw, int *dh) {
  File f = LittleFS.open(path, "r");
  if (!f) {
    *x = *y = *dw = *dh = 0;
    return;
  }

  SpriteHeader hd = read_header(f);
  f.close();

  screen_rect(hd, scale, x, y, dw, dh);
}

void rect_union(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh,
                int *ux, int *uy, int *uw, int *uh) {
  if (aw <= 0 || ah <= 0) {
    *ux = bx;
    *uy = by;
    *uw = bw;
    *uh = bh;
    return;
  }

  int left = min(ax, bx);
  int top = min(ay, by);
  int right = max(ax + aw, bx + bw);
  int bottom = max(ay + ah, by + bh);

  *ux = left;
  *uy = top;
  *uw = right - left;
  *uh = bottom - top;
}

void sprite_draw_frame_buffered(const char *path, int frameIndex, float scale,
                                int prev_x, int prev_y, int prev_dw,
                                int prev_dh, int *out_x, int *out_y,
                                int *out_dw, int *out_dh) {
  File f = LittleFS.open(path, "r");
  if (!f) {
    Serial.printf("Impossibile aprire: %s\n", path);
    return;
  }

  SpriteHeader hd = read_header(f);
  uint16_t w = hd.w;
  uint16_t h = hd.h;

  int maskBytes = (w * h + 7) / 8;
  int pixelBytes = w * h * 2;
  int frameBytes = maskBytes + pixelBytes;

  f.seek(SPRITE_HEADER_SIZE + frameIndex * frameBytes);

  uint8_t *mask = new uint8_t[maskBytes];
  uint8_t *pixels = new uint8_t[pixelBytes];
  f.read(mask, maskBytes);
  f.read(pixels, pixelBytes);
  f.close();

  int x, y, dw, dh;
  screen_rect(hd, scale, &x, &y, &dw, &dh);

  int ux, uy, uw, uh;
  rect_union(prev_x, prev_y, prev_dw, prev_dh, x, y, dw, dh, &ux, &uy, &uw,
             &uh);

  uint16_t *buffer = new uint16_t[uw * uh];
  memset(buffer, 0, uw * uh * sizeof(uint16_t)); // 0 = nero in RGB565

  for (int dy = 0; dy < dh; dy++) {
    int sy = (int)(dy / scale);
    for (int dx = 0; dx < dw; dx++) {
      int sx = (int)(dx / scale);
      int i = sy * w + sx;
      bool opaque = (mask[i / 8] >> (i % 8)) & 1;
      if (opaque) {
        uint16_t color = pixels[i * 2] | (pixels[i * 2 + 1] << 8);
        int buf_x = (x - ux) + dx;
        int buf_y = (y - uy) + dy;
        buffer[buf_y * uw + buf_x] = color;
      }
    }
  }

  gfx->draw16bitRGBBitmap(ux, uy, buffer, uw, uh);

  delete[] mask;
  delete[] pixels;
  delete[] buffer;

  *out_x = x;
  *out_y = y;
  *out_dw = dw;
  *out_dh = dh;
}
