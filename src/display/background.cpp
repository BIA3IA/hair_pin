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
  const uint16_t HEART_COLOR = 0xB351; // rosa morbido
  const uint16_t STAR_COLOR = 0x9191; // fucsia smorzato
  for (int i = 0; i < COUNT; i++) {
    auto &p = particles[i];
    p.size = i % 2 ? 14 : 16;
    // quattro forme ai lati e due dietro Bianca
    const int widths[] = {16, 16, 84, 84, 42, 58};
    const int heights[] = {22, 70, 30, 78, 28, 52};
    p.anchor_x = gfx->width() * widths[i] / 100 - p.size / 2;
    p.anchor_y = gfx->height() * heights[i] / 100 - p.size / 2;
    p.angle_x = random(6283) * 0.001f;
    p.angle_y = random(6283) * 0.001f;
    int n = p.size - 1;
    bool heart = i % 3 == 0 || i == 5;
    p.color = heart ? HEART_COLOR : STAR_COLOR;
    if (!heart) {
      // stella a cinque punte in un solo tratto
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

void compose(uint16_t *buffer, int x, int y, int w, int h) {
  memset(buffer, 0, (size_t)w * h * sizeof(uint16_t));
  // al contrario: se due forme si sovrappongono vince la prima
  for (int i = COUNT - 1; i >= 0; i--) {
    const auto &p = particles[i];
    int x0 = max(x, p.x), x1 = min(x + w, p.x + p.size);
    int y0 = max(y, p.y), y1 = min(y + h, p.y + p.size);
    for (int sy = y0; sy < y1; sy++) {
      uint32_t bits = p.rows[sy - p.y];
      uint16_t *out = buffer + (sy - y) * w - x;
      for (int sx = x0; sx < x1; sx++)
        if ((bits >> (sx - p.x)) & 1)
          out[sx] = p.color;
    }
  }
}

constexpr int PATCH_MAX = MAX_SIZE * 2;

void push(int x, int y, int w, int h) {
  int left = max(0, x), top = max(0, y);
  int right = min((int)gfx->width(), x + w);
  int bottom = min((int)gfx->height(), y + h);
  w = right - left;
  h = bottom - top;
  if (w <= 0 || h <= 0)
    return;
  static uint16_t buf[PATCH_MAX * PATCH_MAX];
  compose(buf, left, top, w, h);
  gfx->draw16bitRGBBitmap(left, top, buf, w, h);
}

bool overlaps(int x, int y, int w, int h, int bx, int by, int bw, int bh) {
  return bw > 0 && bh > 0 && x < bx + bw && x + w > bx && y < by + bh &&
         y + h > by;
}

// ridisegna il rettangolo in al massimo 4 pezzi, saltando la zona di Bianca
void patch(int x, int y, int w, int h, int bx, int by, int bw, int bh) {
  if (!overlaps(x, y, w, h, bx, by, bw, bh)) {
    push(x, y, w, h);
    return;
  }
  int mid_top = max(y, by), mid_bottom = min(y + h, by + bh);
  push(x, y, w, mid_top - y);
  push(x, mid_bottom, w, y + h - mid_bottom);
  push(x, mid_top, min(x + w, bx) - x, mid_bottom - mid_top);
  int right = max(x, bx + bw);
  push(right, mid_top, x + w - right, mid_bottom - mid_top);
}
} // namespace

bool background_update(int bx, int by, int bw, int bh) {
  unsigned long now = millis();
  if (!initialized) {
    init_particles();
    last_tick = now;
    for (const auto &p : particles)
      patch(p.x, p.y, p.size, p.size, bx, by, bw, bh);
    return true;
  }
  unsigned long delta = now - last_tick;
  if (delta < FRAME_MS)
    return false;
  last_tick = now;
  bool behind = false;
  for (int i = 0; i < COUNT; i++) {
    auto &p = particles[i];
    int old_x = p.x, old_y = p.y;
    // oscillano sul posto, un giro ogni 20-30 s
    p.angle_x = fmodf(p.angle_x + delta * (0.00022f + i * 0.00002f), 2 * PI);
    p.angle_y = fmodf(p.angle_y + delta * (0.00019f + i * 0.000015f), 2 * PI);
    p.x = p.anchor_x + (int)lroundf(DRIFT_X * sinf(p.angle_x));
    p.y = p.anchor_y + (int)lroundf(DRIFT_Y * sinf(p.angle_y));
    if (p.x == old_x && p.y == old_y)
      continue;
    int x = min(old_x, p.x), y = min(old_y, p.y);
    int w = max(old_x, p.x) + p.size - x, h = max(old_y, p.y) + p.size - y;
    if (w <= PATCH_MAX && h <= PATCH_MAX) {
      patch(x, y, w, h, bx, by, bw, bh);
    } else {
      patch(old_x, old_y, p.size, p.size, bx, by, bw, bh);
      patch(p.x, p.y, p.size, p.size, bx, by, bw, bh);
    }
    behind |= overlaps(x, y, w, h, bx, by, bw, bh);
  }
  return behind;
}

void background_compose(uint16_t *buffer, int x, int y, int w, int h) {
  if (!initialized)
    init_particles();
  compose(buffer, x, y, w, h);
}
