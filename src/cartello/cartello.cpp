#include "cartello.h"
#include "../display/display.h"
#include <U8g2lib.h>

enum class Kind : uint8_t { PLAIN, STEPS };

struct Phrase {
  const char *text;
  int8_t from_hour, to_hour; // -1 = a qualsiasi ora
  int8_t mood = -1;          // -1 = con qualsiasi umore
  Icon icon = Icon::NONE;
  Kind kind = Kind::PLAIN;
};

const int8_t FELICE = (int8_t)Mood::FELICE;
const int8_t GIU = (int8_t)Mood::GIU;
const int8_t TRISTE = (int8_t)Mood::TRISTE;

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
    {"Nanna", 23, 7, -1, Icon::MOON},
    {"Che sonno", 22, 7, -1, Icon::MOON},
    {"", -1, -1, FELICE, Icon::HEART},
    {"Ti voglio bene", -1, -1, FELICE, Icon::HEART},
    {"Che bello", -1, -1, FELICE, Icon::STAR},
    {"Giornata top", -1, -1, FELICE, Icon::STAR},
    {"EVVIVA", -1, -1, FELICE},
    {"Mi annoio", -1, -1, GIU},
    {"Coccole?", -1, -1, GIU, Icon::HEART},
    {"Uff...", -1, -1, GIU},
    {"Mi ignori?", -1, -1, TRISTE},
    {"Mi manchi", -1, -1, TRISTE, Icon::HEART},
    {"Sola soletta", -1, -1, TRISTE},
    {"passi", -1, -1, -1, Icon::STEPS, Kind::STEPS},
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
static char steps_text[24];

const unsigned long STEPS_MIN = 1000; // sotto non vale la pena vantarsene

// icone 8x8 in pixel art, un byte per riga (bit alto = pixel a sinistra)
const uint8_t ICON_BITS[][8] = {
    {},                                                     // NONE
    {0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00},       // HEART
    {0x60, 0xE0, 0xE0, 0x66, 0x07, 0x07, 0x06, 0x00},       // STEPS
    {0x1C, 0x30, 0x60, 0x60, 0x60, 0x30, 0x1C, 0x00},       // MOON
    {0x10, 0x10, 0xFE, 0x7C, 0x38, 0x6C, 0x44, 0x00},       // STAR
};
const uint16_t ICON_COLORS[] = {0, 0xD948, SIGN_TEXT_COLOR, 0x5AD4, 0xE5A5};
const int ICON_TEXT_SCALE = 2; // accanto al testo: 16x16 px
const int ICON_GAP = 3;

static void draw_icon(Icon icon, int x, int y, int scale) {
  const uint8_t *bits = ICON_BITS[(int)icon];
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (bits[row] & (0x80 >> col)) {
        gfx->fillRect(x + col * scale, y + row * scale, scale, scale,
                      ICON_COLORS[(int)icon]);
      }
    }
  }
}

static const char *phrase_text(const Phrase &p, unsigned long steps) {
  if (p.kind != Kind::STEPS) {
    return p.text;
  }
  if (steps >= 1000) {
    snprintf(steps_text, sizeof(steps_text), "%lu.%03lu\n%s", steps / 1000,
             steps % 1000, p.text);
  } else {
    snprintf(steps_text, sizeof(steps_text), "%lu\n%s", steps, p.text);
  }
  return steps_text;
}

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

SignContent sign_pick_phrase(int hour, Mood mood, unsigned long steps) {
  int candidates[PHRASE_COUNT];
  int n = 0;
  auto collect = [&](auto wanted) {
    n = 0;
    for (int i = 0; i < PHRASE_COUNT; i++) {
      const Phrase &p = PHRASES[i];
      bool usable = i != last_phrase && fits_hour(p, hour) &&
                    (p.mood < 0 || p.mood == (int8_t)mood) &&
                    (p.kind != Kind::STEPS || steps >= STEPS_MIN);
      if (usable && wanted(p)) {
        candidates[n++] = i;
      }
    }
  };

  // passi del giorno, umore e ora: ognuno ha la sua occasione, poi le altre
  if (steps >= STEPS_MIN && random(4) == 0) {
    collect([](const Phrase &p) { return p.kind == Kind::STEPS; });
  }
  if (n == 0 && mood != Mood::NORMALE && random(2) == 0) {
    collect([](const Phrase &p) { return p.mood >= 0; });
  }
  if (n == 0 && random(2) == 0) {
    collect([](const Phrase &p) { return p.from_hour >= 0; });
  }
  if (n == 0) {
    collect([](const Phrase &p) { return p.kind == Kind::PLAIN; });
  }
  if (n == 0) {
    return {"BOH", Icon::NONE};
  }

  last_phrase = candidates[random(n)];
  const Phrase &p = PHRASES[last_phrase];
  return {phrase_text(p, steps), p.icon};
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

static void draw_text(const char *text, int x, int y, int w, int h);

void sign_draw(const SignContent &c, int x, int y, int w, int h) {
  if (c.icon == Icon::NONE) {
    draw_text(c.text, x, y, w, h);
    return;
  }
  if (c.text[0] == '\0') { // solo l'icona, grande e in mezzo
    int scale = max(1, min(min(w, h) - 8, 40) / 8);
    draw_icon(c.icon, x + (w - 8 * scale) / 2, y + (h - 8 * scale) / 2, scale);
    return;
  }
  int icon_w = 8 * ICON_TEXT_SCALE;
  draw_icon(c.icon, x + SIDE_MARGIN, y + (h - icon_w) / 2, ICON_TEXT_SCALE);
  int shift = icon_w + ICON_GAP; // il margine sinistro del testo resta
  draw_text(c.text, x + shift, y, w - shift, h);
}

static void draw_text(const char *text, int x, int y, int w, int h) {
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

  // centra sui pixel veri della scritta, non sul riquadro del font
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
  int icon_shift = 8 * ICON_TEXT_SCALE + ICON_GAP;
  for (int i = 0; i < PHRASE_COUNT; i++) {
    const Phrase &p = PHRASES[i];
    const char *text = phrase_text(p, 88888); // il numero piu' largo possibile
    int text_w = w - 2 * SIDE_MARGIN - (p.icon != Icon::NONE ? icon_shift : 0);
    if (text[0] && !wrap(text, SMALL, text_w, h, lines, &line_count)) {
      Serial.printf("Frase troppo lunga per il cartoncino: %s\n", text);
    }
  }
}
