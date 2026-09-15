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

void sprite_draw_frame(const char *path, int frameIndex, int x, int y) {
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

  f.seek(6 + frameIndex * frameBytes); // salta all'inizio del frame richiesto

  uint8_t *mask = new uint8_t[maskBytes];
  uint8_t *pixels = new uint8_t[pixelBytes];
  f.read(mask, maskBytes);
  f.read(pixels, pixelBytes);
  f.close();

  gfx->startWrite();
  for (int i = 0; i < w * h; i++) {
    bool opaque = (mask[i / 8] >> (i % 8)) & 1;
    if (opaque) {
      uint16_t color = pixels[i * 2] | (pixels[i * 2 + 1] << 8);
      int px = i % w;
      int py = i / w;
      gfx->writePixel(x + px, y + py, color);
    }
  }
  gfx->endWrite();

  delete[] mask;
  delete[] pixels;
}
