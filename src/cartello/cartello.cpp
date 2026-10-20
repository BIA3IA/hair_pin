#include "cartello.h"
#include "../display/display.h"
#include <U8g2lib.h>

struct Phrase {
  const char *text;
  int8_t from_hour, to_hour; // -1 = a qualsiasi ora
};

const Phrase PHRASES[] = {
    {"Ho fame", 12, 14},
    {"Ho fame", 19, 21},
    {"CAFFÈ", 7, 11},
    {"CAFFÈ", 14, 16},
    {"Datemi un caffè", 7, 11},
    {"Datemi un caffè", 14, 16},
    {"Sono stanca", 22, 4},
    {"Che fatica", 22, 4},
    {"Ancora viva", 23, 4},
    {"SOPRAVVIVO", 23, 4},

    {"RYUK", -1, -1},
    {"Dov'è Ryuk", -1, -1},
    {"GATTO", -1, -1},
    {"ESISTO?", -1, -1},
    {"Dove sono?", -1, -1},
    {"Chi sono?", -1, -1},
    {"Aiuto boh", -1, -1},
    {"BOH", -1, -1},
    {"EH?", -1, -1},
    {"PERCHÉ?", -1, -1},
    {"Ma quindi?", -1, -1},
    {"Che ansia", -1, -1},
    {"ANSIA", -1, -1},
    {"No vabbè", -1, -1},
    {"Non so", -1, -1},
    {"Mi dissocio", -1, -1},
    {"Sto pensando", -1, -1},
    {"FORSE", -1, -1},
    {"Forse no", -1, -1},
    {"Io sparisco", -1, -1},
    {"Vi osservo", -1, -1},
    {"Vi sto osservando", -1, -1},
    {"Ti vedo", -1, -1},
    {"Pessima idea", -1, -1},
    {"Scelta dubbia", -1, -1},
    {"Brutta idea", -1, -1},
    {"Che sbatti", -1, -1},
    {"Ho perso", -1, -1},
    {"Dov'è finito", -1, -1},
    {"Dove l'ho messo", -1, -1},
    {"Dove ho messo?", -1, -1},
    {"Ho dimenticato qualcosa?", -1, -1},
    {"Ero convinta", -1, -1},
    {"Aspetta un attimo", -1, -1},
    {"Un secondo", -1, -1},
    {"Che giorno è", -1, -1},
    {"Io boh", -1, -1},
    {"PANICO", -1, -1},
    {"GATTO.EXE", -1, -1},
    {"BIANCA.EXE", -1, -1},
    {"System error", -1, -1},
    {"LOADING...", -1, -1},
    {"BUFFERING", -1, -1},
    {"PENSIERI", -1, -1},
    {"OVERTHINKING", -1, -1},
    {"Quasi bene", -1, -1},
    {"MIAO", -1, -1},
    {"Boh raga", -1, -1},
    {"MAH", -1, -1},
    {"UFFA", -1, -1},
    {"Risolvo dopo", -1, -1},
};
const int PHRASE_COUNT = sizeof(PHRASES) / sizeof(PHRASES[0]);

const uint16_t SIGN_TEXT_COLOR = 0x18C3; // quasi nero
const int CHAR_W = 8;                    // unifont: lettere larghe 8 px
const int LINE_H = 16;
const int MAX_LINES = 4;
const int MAX_LINE_BYTES = 48;

static int last_phrase = -1;

static bool fits_hour(const Phrase &p, int hour) {
  if (p.from_hour < 0) {
    return true;
  }
  if (hour < 0) {
    return false;
  }
  if (p.from_hour <= p.to_hour) {
    return hour >= p.from_hour && hour < p.to_hour;
  }
  return hour >= p.from_hour || hour < p.to_hour; // fascia che passa la mezzanotte
}

const char *sign_pick_phrase(int hour) {
  int candidates[PHRASE_COUNT];
  int n = 0;

  // se c'e' una frase legata all'ora attuale, una volta su due esce quella
  if (random(2) == 0) {
    for (int i = 0; i < PHRASE_COUNT; i++) {
      if (PHRASES[i].from_hour >= 0 && fits_hour(PHRASES[i], hour) &&
          i != last_phrase) {
        candidates[n++] = i;
      }
    }
  }
  if (n == 0) {
    for (int i = 0; i < PHRASE_COUNT; i++) {
      if (fits_hour(PHRASES[i], hour) && i != last_phrase) {
        candidates[n++] = i;
      }
    }
  }

  last_phrase = candidates[random(n)];
  return PHRASES[last_phrase].text;
}

// lettere vere, non byte: "è" occupa 2 byte ma e' una lettera sola
static int utf8_len(const char *s) {
  int n = 0;
  for (; *s; s++) {
    if ((*s & 0xC0) != 0x80) {
      n++;
    }
  }
  return n;
}

void sign_draw_text(const char *text, int x, int y, int w, int h) {
  char lines[MAX_LINES][MAX_LINE_BYTES] = {};
  int line_count = 1;
  int max_chars = w / CHAR_W;

  // va a capo tra una parola e l'altra
  const char *p = text;
  while (*p) {
    const char *space = strchr(p, ' ');
    int word_bytes = space ? space - p : strlen(p);
    char word[MAX_LINE_BYTES] = {};
    strncpy(word, p, min(word_bytes, MAX_LINE_BYTES - 1));

    char *line = lines[line_count - 1];
    if (line[0] && utf8_len(line) + 1 + utf8_len(word) > max_chars &&
        line_count < MAX_LINES) {
      line_count++;
      line = lines[line_count - 1];
    }
    if (line[0]) {
      strncat(line, " ", MAX_LINE_BYTES - strlen(line) - 1);
    }
    strncat(line, word, MAX_LINE_BYTES - strlen(line) - 1);

    p = space ? space + 1 : p + word_bytes;
  }

  gfx->setFont(u8g2_font_unifont_tf);
  gfx->setUTF8Print(true);
  gfx->setTextColor(SIGN_TEXT_COLOR);

  int top = y + (h - line_count * LINE_H) / 2;
  for (int i = 0; i < line_count; i++) {
    int line_w = utf8_len(lines[i]) * CHAR_W;
    gfx->setCursor(x + (w - line_w) / 2, top + i * LINE_H + 13); // 13 = linea di base
    gfx->print(lines[i]);
  }
}
