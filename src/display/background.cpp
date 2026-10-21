#include "background.h"
#include "display.h"
#include <math.h>

namespace {
constexpr int COUNT = 6;
constexpr int MAX_SIZE = 16;
constexpr unsigned long FRAME_MS = 33;
constexpr int DRIFT_X = 12;
constexpr int DRIFT_Y = 18;
struct Particle {
  int x, y, size, anchor_x, anchor_y;
  float angle_x, angle_y;
  uint16_t color;
  uint32_t rows[MAX_SIZE] = {};
};
Particle particles[COUNT];
bool initialized = false;
unsigned long last_tick = 0;

void line(Particle &p, int x, int y, int end_x, int end_y) {
  int dx = abs(end_x - x), sx = x < end_x ? 1 : -1;
  int dy = -abs(end_y - y), sy = y < end_y ? 1 : -1;
  int error = dx + dy;
  while (true) {
    p.rows[y] |= 1UL << x;
    if (x == end_x && y == end_y)
      break;
    int twice = 2 * error;
    if (twice >= dy) {
      error += dy;
      x += sx;
    }
    if (twice <= dx) {
      error += dx;
      y += sy;
    }
  }
}

void init_particles() {
  const uint16_t colors[] = {0x596e, 0x7a0c,
                             0x61cf}; // viola e rosa malva della sprite
  for (int i = 0; i < COUNT; i++) {
    auto &p = particles[i];
    p.size = i % 2 ? 14 : 16;
    // Quattro forme ai lati e due dietro Bianca, visibili quando si sposta
    // o cade. Il renderer della sprite le copre solo nei pixel opachi.
    const int widths[] = {16, 16, 84, 84, 42, 58};
    const int heights[] = {22, 70, 30, 78, 28, 52};
    p.anchor_x = gfx->width() * widths[i] / 100 - p.size / 2;
    p.anchor_y = gfx->height() * heights[i] / 100 - p.size / 2;
    p.angle_x = random(6283) * 0.001f;
    p.angle_y = random(6283) * 0.001f;
    p.color = colors[i % 3];
    int n = p.size - 1;
    bool heart = i % 3 == 0 || i == 5;
    if (!heart) {
      // Un solo tratto chiuso: alto, basso-destra, sinistra,
      // destra, basso-sinistra. Pentagramma senza cerchio.
      int vx[] = {n / 2, n * 4 / 5, 0, n, n / 5};
      int vy[] = {0, n, n * 2 / 5, n * 2 / 5, n};
      for (int j = 0; j < 5; j++)
        line(p, vx[j], vy[j], vx[(j + 1) % 5], vy[(j + 1) % 5]);
    } else {
      int vx[] = {n / 2, n / 4, 0, 0, n / 2, n, n, n * 3 / 4, n / 2};
      int vy[] = {n / 5, 0, n / 5, n * 2 / 5, n, n * 2 / 5, n / 5, 0, n / 5};
      for (int j = 0; j < 8; j++)
        line(p, vx[j], vy[j], vx[j + 1], vy[j + 1]);
    }
    p.x = p.anchor_x + (int)lroundf(DRIFT_X * sinf(p.angle_x));
    p.y = p.anchor_y + (int)lroundf(DRIFT_Y * sinf(p.angle_y));
  }
  initialized = true;
}

uint16_t pixel(int x, int y) {
  for (const auto &p : particles) {
    int dx = x - p.x, dy = y - p.y;
    if (dx >= 0 && dx < p.size && dy >= 0 && dy < p.size &&
        (p.rows[dy] & (1UL << dx)))
      return p.color;
  }
  return 0;
}

// Trasferisce piccole righe gia' composte: nessun erase visibile e nessun
// framebuffer a schermo intero. La regione di Bianca/cartello e' protetta.
void patch(int x, int y, int w, int h, int bx, int by, int bw, int bh) {
  int left = max(0, x), right = min((int)gfx->width(), x + w);
  uint16_t row[MAX_SIZE + 4];
  for (int sy = max(0, y); sy < min((int)gfx->height(), y + h); sy++) {
    int start = left;
    while (start < right) {
      if (sy >= by && sy < by + bh && start >= bx && start < bx + bw) {
        start = min(right, bx + bw);
        continue;
      }
      int end = right;
      if (sy >= by && sy < by + bh && start < bx)
        end = min(end, bx);
      int count = min(end - start, MAX_SIZE + 4);
      for (int j = 0; j < count; j++)
        row[j] = pixel(start + j, sy);
      gfx->draw16bitRGBBitmap(start, sy, row, count, 1);
      start += count;
    }
  }
}
} // namespace

void background_update(int bx, int by, int bw, int bh) {
  unsigned long now = millis();
  if (!initialized) {
    init_particles();
    last_tick = now;
    for (const auto &p : particles)
      patch(p.x, p.y, p.size, p.size, bx, by, bw, bh);
    return;
  }
  unsigned long delta = now - last_tick;
  if (delta < FRAME_MS)
    return;
  last_tick = now;
  int old_x[COUNT], old_y[COUNT];
  for (int i = 0; i < COUNT; i++) {
    auto &p = particles[i];
    old_x[i] = p.x;
    old_y[i] = p.y;
    // Oscillazioni locali visibili, con periodi di circa 20-30 secondi.
    // Nessun flusso verticale, ingresso dai bordi o riciclo delle forme.
    p.angle_x = fmodf(p.angle_x + delta * (0.00022f + i * 0.00002f), 2 * PI);
    p.angle_y = fmodf(p.angle_y + delta * (0.00019f + i * 0.000015f), 2 * PI);
    p.x = p.anchor_x + (int)lroundf(DRIFT_X * sinf(p.angle_x));
    p.y = p.anchor_y + (int)lroundf(DRIFT_Y * sinf(p.angle_y));
  }
  for (int i = 0; i < COUNT; i++) {
    const auto &p = particles[i];
    patch(old_x[i], old_y[i], p.size, p.size, bx, by, bw, bh);
    patch(p.x, p.y, p.size, p.size, bx, by, bw, bh);
  }
}

void background_compose(uint16_t *buffer, int x, int y, int w, int h) {
  if (!initialized)
    init_particles();
  for (int sy = 0; sy < h; sy++)
    for (int sx = 0; sx < w; sx++)
      buffer[sy * w + sx] = pixel(x + sx, y + sy);
}
