#include "cartello.h"
#include "../display/display.h"
#include <U8g2lib.h>

struct Phrase {
  const char *text;
  int8_t from_hour, to_hour; // -1 = a qualsiasi ora
};

const Phrase PHRASES[] = {
    {"FAME", 12, 14},
    {"FAME", 19, 21},
    {"CAFFÈ", -1, -1},
    {"Sono stanca", -1, -1},
    {"Che fatica", -1, -1},
    {"Ancora viva", -1, -1},
    {"SOPRAVVIVO", -1, -1},
    {"RYUK", -1, -1},
    {"Dov'è Ryuk?", -1, -1},
    {"GATTO?", -1, -1},
    {"ESISTO?", -1, -1},
    {"Dove sono?", -1, -1},
    {"Chi sono?", -1, -1},
    {"EH?", -1, -1},
    {"PERCHÉ?", -1, -1},
    {"Ma quindi?", -1, -1},
    {"ANSIA", -1, -1},
    {"vabbè", -1, -1},
    {"Non so", -1, -1},
    {"FORSE?", -1, -1},
    {"FORSE NO?", -1, -1},
    {"SPARISCO", -1, -1},
    {"VI OSSERVO", -1, -1},
    {"TI VEDO", -1, -1},
    {"Pessima idea", -1, -1},
    {"Scelta dubbia", -1, -1},
    {"Brutta idea", -1, -1},
    {"Che sbatti", -1, -1},
    {"Dov'è finito", -1, -1},
    {"Dove ho messo?", -1, -1},
    {"Ho dimenticato qualcosa?", -1, -1},
    {"Ero convinta", -1, -1},
    {"Aspetta un attimo", -1, -1},
    {"Un secondo", -1, -1},
    {"Che giorno è", -1, -1},
    {"BOH", -1, -1},
    {"PANICO", -1, -1},
    {"GATTO.EXE", -1, -1},
    {"BIANCA.EXE", -1, -1},
    {"System error", -1, -1},
    {"LOADING...", -1, -1},
    {"BUFFERING", -1, -1},
    {"PENSIERI", -1, -1},
    {"OVER-\nTHINKING", -1, -1},
    {"Quasi bene", -1, -1},
    {"MIAO", -1, -1},
    {"MAH", -1, -1},
    {"UFFA", -1, -1},
    {"Risolvo dopo", -1, -1},
};
const int PHRASE_COUNT = sizeof(PHRASES) / sizeof(PHRASES[0]);

const uint16_t SIGN_TEXT_COLOR = 0x18C3; // quasi nero
const int MAX_LINES = 4;
const int MAX_LINE_BYTES = 48;
const int SIDE_MARGIN = 4; // px liberi a destra e a sinistra della scritta

struct TextStyle {
  const uint8_t *font;
  uint8_t scale;
  int line_h, baseline;
  int bold_x, bold_y; // ripassa la scritta spostata di questi px, per ingrossarla
};

// si prova dal piu' grande al piu' piccolo, vince il primo in cui la frase ci sta
// enorme: 5x8 ingrandito 3 volte, una riga sola (BOH, MIAO, GATTO...)
const TextStyle HUGE = {u8g2_font_5x8_tf, 3, 24, 21, 1, 1};
// grande: 5x8 ingrandito 2 volte, due righe
const TextStyle BIG = {u8g2_font_5x8_tf, 2, 16, 14, 1, 1};
// medio: grassetto vero, due righe
const TextStyle MEDIUM = {u8g2_font_helvB10_tf, 1, 16, 13, 1, 0};
// piccolo: grassetto vero, tre righe, per le frasi lunghe
const TextStyle SMALL = {u8g2_font_helvB08_tf, 1, 11, 10, 0, 0};

const TextStyle *STYLES[] = {&HUGE, &BIG, &MEDIUM, &SMALL};
const int STYLE_COUNT = sizeof(STYLES) / sizeof(STYLES[0]);

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

static void use_style(const TextStyle &st) {
  gfx->setFont(st.font);
  gfx->setTextSize(st.scale);
  gfx->setUTF8Print(true);
}

static int text_width(const char *s, const TextStyle &st) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return w + st.bold_x;
}

// va a capo tra una parola e l'altra; false se in quello stile non ci sta
static bool wrap(const char *text, const TextStyle &st, int w, int h,
                 char lines[MAX_LINES][MAX_LINE_BYTES], int *line_count) {
  use_style(st);
  int max_lines = min(MAX_LINES, h / st.line_h);
  memset(lines, 0, MAX_LINES * MAX_LINE_BYTES);
  *line_count = 1;
  bool fits = true;

  // "\n" nella frase = vai a capo qui (anche dentro una parola: "OVER-\nTHINKING")
  bool force_break = false;
  const char *p = text;
  while (*p) {
    int word_bytes = strcspn(p, " \n");
    char separator = p[word_bytes];
    char word[MAX_LINE_BYTES] = {};
    strncpy(word, p, min(word_bytes, MAX_LINE_BYTES - 1));

    char *line = lines[*line_count - 1];
    char candidate[MAX_LINE_BYTES * 2];
    snprintf(candidate, sizeof(candidate), "%s%s%s", line, line[0] ? " " : "",
             word);

    if (line[0] && (force_break || text_width(candidate, st) > w)) {
      if (*line_count == max_lines) {
        fits = false;
        strncat(line, " ", MAX_LINE_BYTES - strlen(line) - 1);
        strncat(line, word, MAX_LINE_BYTES - strlen(line) - 1);
      } else {
        (*line_count)++;
        strncpy(lines[*line_count - 1], word, MAX_LINE_BYTES - 1);
      }
    } else {
      strncpy(line, candidate, MAX_LINE_BYTES - 1);
    }

    force_break = separator == '\n';
    p += word_bytes + (separator ? 1 : 0);
  }

  for (int i = 0; i < *line_count; i++) {
    if (text_width(lines[i], st) > w) {
      fits = false; // una parola sola piu' lunga della riga
    }
  }
  return fits;
}

void sign_draw_text(const char *text, int x, int y, int w, int h) {
  char lines[MAX_LINES][MAX_LINE_BYTES];
  int line_count;

  const TextStyle *st = &SMALL;
  for (int i = 0; i < STYLE_COUNT; i++) {
    if (wrap(text, *STYLES[i], w - 2 * SIDE_MARGIN, h, lines, &line_count)) {
      st = STYLES[i];
      break;
    }
  }
  if (st == &SMALL) {
    wrap(text, SMALL, w - 2 * SIDE_MARGIN, h, lines, &line_count); // anche se non ci sta del tutto
  }

  use_style(*st);
  gfx->setTextColor(SIGN_TEXT_COLOR);

  // misura i pixel veri di ogni riga (rispetto alla sua linea di base),
  // cosi' il blocco di testo si centra davvero, anche in verticale
  int16_t ink_x[MAX_LINES];
  uint16_t ink_w[MAX_LINES];
  int ink_top = INT16_MAX, ink_bottom = INT16_MIN;
  for (int i = 0; i < line_count; i++) {
    int16_t y1;
    uint16_t ink_h;
    gfx->getTextBounds(lines[i], 0, 0, &ink_x[i], &y1, &ink_w[i], &ink_h);
    ink_top = min(ink_top, i * st->line_h + y1);
    ink_bottom = max(ink_bottom, i * st->line_h + y1 + ink_h + st->bold_y);
  }
  int first_baseline = y + (h - (ink_bottom - ink_top)) / 2 - ink_top;

  for (int i = 0; i < line_count; i++) {
    int line_x = x + (w - (ink_w[i] + st->bold_x)) / 2 - ink_x[i];
    int line_y = first_baseline + i * st->line_h;
    for (int dx = 0; dx <= st->bold_x; dx++) {
      for (int dy = 0; dy <= st->bold_y; dy++) {
        gfx->setCursor(line_x + dx, line_y + dy);
        gfx->print(lines[i]);
      }
    }
  }
}

void sign_check_phrases(int w, int h) {
  char lines[MAX_LINES][MAX_LINE_BYTES];
  int line_count;
  for (int i = 0; i < PHRASE_COUNT; i++) {
    if (!wrap(PHRASES[i].text, SMALL, w - 2 * SIDE_MARGIN, h, lines, &line_count)) {
      Serial.printf("Frase troppo lunga per il cartoncino: %s\n", PHRASES[i].text);
    }
  }
}
