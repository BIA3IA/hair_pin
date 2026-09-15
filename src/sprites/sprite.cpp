#include "sprite.h"
#include "../display/display.h"
#include <LittleFS.h>

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

  uint8_t header[6];
  f.read(header, 6);
  f.close();

  uint16_t width =
      header[0] | (header[1] << 8); // little-endian: byte basso prima
  uint16_t height = header[2] | (header[3] << 8);
  uint16_t frameCount = header[4] | (header[5] << 8);
  Serial.printf("width=%d height=%d frames=%d\n", width, height, frameCount);
}

void sprite_draw_frame(const char *path, int frameIndex, float scale) {
  File f = LittleFS.open(path, "r");
  if (!f)
    return;

  uint8_t header[6];
  f.read(header, 6);
  uint16_t w = header[0] | (header[1] << 8);
  uint16_t h = header[2] | (header[3] << 8);

  int maskBytes = (w * h + 7) / 8;
  int pixelBytes = w * h * 2;
  int frameBytes = maskBytes + pixelBytes;

  f.seek(6 + frameIndex * frameBytes);

  uint8_t *mask = new uint8_t[maskBytes];
  uint8_t *pixels = new uint8_t[pixelBytes];
  f.read(mask, maskBytes);
  f.read(pixels, pixelBytes);
  f.close();

  int dw = (int)(w * scale); // larghezza finale a schermo
  int dh = (int)(h * scale); // altezza finale a schermo

  int x = (SCREEN_WIDTH - dw) / 2;
  int y = (SCREEN_HEIGHT - dh) / 2;

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
