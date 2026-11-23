#include "sprite.h"
#include "../display/background.h"
#include "../display/display.h"
#include <LittleFS.h>
#include <new>
#include <utility>

namespace {
File file;
const char *file_path = nullptr;
SpriteHeader header;

// buffer che crescono soltanto: niente new/delete a ogni frame
uint8_t *mask = nullptr;
uint16_t *pixels = nullptr;
uint16_t *canvas = nullptr, *shown = nullptr;
size_t mask_cap = 0, pixels_cap = 0, canvas_cap = 0, shown_cap = 0;
// zona dello schermo che corrisponde a "shown"; w = 0 se non valida
int shown_x = 0, shown_y = 0, shown_w = 0, shown_h = 0;
int cached_frame = -1;

int16_t src_col[SCREEN_WIDTH * 2];
int16_t src_row[SCREEN_HEIGHT * 2];

template <typename T> bool reserve(T *&buf, size_t &cap, size_t count) {
  if (count <= cap)
    return true;
  delete[] buf;
  buf = new (std::nothrow) T[count];
  cap = buf ? count : 0;
  return buf != nullptr;
}

SpriteHeader read_header(File &f) {
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

// tiene aperto l'ultimo file: riaprirlo a ogni frame costa millisecondi
bool open_sprite(const char *path) {
  if (file && file_path &&
      (file_path == path || strcmp(file_path, path) == 0))
    return true;
  if (file)
    file.close();
  file_path = nullptr;
  cached_frame = -1;
  file = LittleFS.open(path, "r");
  if (!file) {
    Serial.printf("Impossibile aprire: %s\n", path);
    return false;
  }
  header = read_header(file);
  file_path = path;
  return true;
}

// centra la tela originale, non il ritaglio
void screen_rect(const SpriteHeader &hd, float scale, int *x, int *y, int *dw,
                 int *dh) {
  *dw = (int)(hd.w * scale);
  *dh = (int)(hd.h * scale);
  *x = (SCREEN_WIDTH - (int)(hd.canvas_w * scale)) / 2 +
       (int)(hd.offset_x * scale);
  *y = (SCREEN_HEIGHT - (int)(hd.canvas_h * scale)) / 2 +
       (int)(hd.offset_y * scale);
}

bool load_frame(int frame) {
  if (frame == cached_frame)
    return true;
  size_t count = (size_t)header.w * header.h;
  size_t mask_bytes = (count + 7) / 8;
  if (!reserve(mask, mask_cap, mask_bytes) ||
      !reserve(pixels, pixels_cap, count)) {
    Serial.println("Memoria finita per il frame!");
    return false;
  }
  file.seek(SPRITE_HEADER_SIZE + frame * (mask_bytes + count * 2));
  file.read(mask, mask_bytes);
  // i pixel sono little-endian come la CPU: si leggono gia' come uint16_t
  file.read((uint8_t *)pixels, count * 2);
  cached_frame = frame;
  return true;
}
// manda solo il tratto cambiato di ogni riga rispetto all'ultimo invio
void push_changes(int left, int top, int w, int h) {
  for (int r = 0; r < h; r++) {
    uint16_t *now = canvas + r * w;
    const uint16_t *before = shown + r * w;
    int first = 0;
    while (first < w && now[first] == before[first])
      first++;
    if (first == w)
      continue;
    int last = w - 1;
    while (now[last] == before[last])
      last--;
    gfx->draw16bitRGBBitmap(left + first, top + r, now + first,
                            last - first + 1, 1);
  }
}
} // namespace

void sprite_invalidate() { shown_w = 0; }

void sprite_init() {
  if (!LittleFS.begin()) {
    Serial.println("Errore montaggio LittleFS!");
    return;
  }

  // buffer gia' grandi abbastanza per qualsiasi sprite: la RAM non si frammenta
  int left = SCREEN_WIDTH, top = SCREEN_HEIGHT, right = 0, bottom = 0;
  File dir = LittleFS.open("/Idle");
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    int x, y, dw, dh;
    screen_rect(read_header(f), SPRITE_SCALE, &x, &y, &dw, &dh);
    left = min(left, max(x, 0));
    top = min(top, max(y, 0));
    right = max(right, min(x + dw, SCREEN_WIDTH));
    bottom = max(bottom, min(y + dh, SCREEN_HEIGHT));
  }
  if (right > left && bottom > top) {
    size_t count = (size_t)(right - left) * (bottom - top);
    reserve(canvas, canvas_cap, count);
    reserve(shown, shown_cap, count);
  }
  if (open_sprite("/Idle/idle_1.bin")) {
    Serial.printf("width=%d height=%d frames=%d canvas=%dx%d offset=%d,%d\n",
                  header.w, header.h, header.frame_count, header.canvas_w,
                  header.canvas_h, header.offset_x, header.offset_y);
  }
}

int sprite_get_frame_count(const char *path) {
  return open_sprite(path) ? header.frame_count : 1;
}

void sprite_canvas_origin(const char *path, int *x, int *y) {
  if (!open_sprite(path)) {
    *x = *y = 0;
    return;
  }
  *x = (SCREEN_WIDTH - (int)(header.canvas_w * SPRITE_SCALE)) / 2;
  *y = (SCREEN_HEIGHT - (int)(header.canvas_h * SPRITE_SCALE)) / 2;
}

void sprite_draw_frame_buffered(const char *path, int frame, int prev_x,
                                int prev_y, int prev_dw, int prev_dh,
                                int *out_x, int *out_y, int *out_dw,
                                int *out_dh) {
  if (!open_sprite(path) || frame < 0 || frame >= header.frame_count ||
      !load_frame(frame))
    return;
  const float scale = SPRITE_SCALE;

  int x, y, dw, dh;
  screen_rect(header, scale, &x, &y, &dw, &dh);
  dw = min(dw, (int)(sizeof(src_col) / sizeof(src_col[0])));
  dh = min(dh, (int)(sizeof(src_row) / sizeof(src_row[0])));

  // unione col rettangolo vecchio, tagliata ai bordi dello schermo
  int left = x, top = y, right = x + dw, bottom = y + dh;
  if (prev_dw > 0 && prev_dh > 0) {
    left = min(left, prev_x);
    top = min(top, prev_y);
    right = max(right, prev_x + prev_dw);
    bottom = max(bottom, prev_y + prev_dh);
  }
  left = max(left, 0);
  top = max(top, 0);
  right = min(right, SCREEN_WIDTH);
  bottom = min(bottom, SCREEN_HEIGHT);
  int uw = right - left, uh = bottom - top;
  if (uw <= 0 || uh <= 0)
    return;

  if (!reserve(canvas, canvas_cap, (size_t)uw * uh)) {
    sprite_invalidate();
    Serial.println("Memoria finita per il buffer!");
    return;
  }
  background_compose(canvas, left, top, uw, uh);

  // pixel sorgente piu' vicino, calcolato una volta per colonna e per riga
  for (int dx = 0; dx < dw; dx++)
    src_col[dx] = (int16_t)(dx / scale);
  for (int dy = 0; dy < dh; dy++)
    src_row[dy] = (int16_t)(dy / scale);

  int dx0 = max(0, left - x), dx1 = min(dw, right - x);
  int dy0 = max(0, top - y), dy1 = min(dh, bottom - y);
  const int w = header.w;
  for (int dy = dy0; dy < dy1; dy++) {
    const int row = src_row[dy] * w;
    uint16_t *out = canvas + (y + dy - top) * uw + (x - left);
    for (int dx = dx0; dx < dx1; dx++) {
      const int i = row + src_col[dx];
      if ((mask[i >> 3] >> (i & 7)) & 1)
        out[dx] = pixels[i];
    }
  }

  if (shown_w == uw && shown_h == uh && shown_x == left && shown_y == top) {
    push_changes(left, top, uw, uh);
  } else {
    gfx->draw16bitRGBBitmap(left, top, canvas, uw, uh);
  }
  std::swap(canvas, shown);
  std::swap(canvas_cap, shown_cap);
  shown_x = left;
  shown_y = top;
  shown_w = uw;
  shown_h = uh;

  *out_x = x;
  *out_y = y;
  *out_dw = dw;
  *out_dh = dh;
}
