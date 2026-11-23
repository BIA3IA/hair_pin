#include "cartello.h"
#include "../display/display.h"
#include <U8g2lib.h>

struct Phrase {
  const char *text;
  int8_t from_hour, to_hour; // -1 = a qualsiasi ora
  int8_t mood = -1;          // -1 = con qualsiasi umore
  Icon icon = Icon::NONE;
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


// icone 8x8 in pixel art, un byte per riga (bit alto = pixel a sinistra)
const uint8_t ICON_BITS[][8] = {
    {},                                                     // NONE
    {0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00},       // HEART
    {0x60, 0xE0, 0xE0, 0x66, 0x07, 0x07, 0x06, 0x00},       // STEPS
    {0x1C, 0x30, 0x60, 0x60, 0x60, 0x30, 0x1C, 0x00},       // MOON
    {0x10, 0x10, 0xFE, 0x7C, 0x38, 0x6C, 0x44, 0x00},       // STAR
    {0x7E, 0x81, 0xAB, 0x81, 0x7E, 0x30, 0x20, 0x00},       // MESSAGE
    {0x3C, 0x24, 0x24, 0x24, 0x24, 0x3C, 0x00, 0x00},       // PHONE
    {0xFF, 0xC3, 0xA5, 0x99, 0x81, 0xFF, 0x00, 0x00},       // MAIL
    {0x18, 0x3C, 0x7E, 0x7E, 0x7E, 0xFF, 0x18, 0x00},       // BELL
};
const uint16_t ICON_COLORS[] = {0,      0xD948, SIGN_TEXT_COLOR, 0x5AD4, 0xE5A5,
                                0x69F4, SIGN_TEXT_COLOR, 0x5AD4, 0xE5A5};
const int ICON_TEXT_SCALE = 2; // accanto al testo: 16x16 px
const int ICON_GAP = 3;

// Simboli monocromatici a risoluzione nativa, 20x20 pixel.
const uint32_t APP_ICON_BITS[3][20] = {
  {0x00000,0x01F80,0x07FE0,0x0E070,0x1C038,0x3B01C,0x3700C,0x67806,0x67006,0x63006,0x61806,0x61D86,0x60FE6,0x307EC,0x301DC,0x36038,0x3E070,0x27FE0,0x01F80,0x00000},
  {0x00000,0x01F80,0x07FE0,0x0FFF0,0x1FFE8,0x3FF0C,0x3F85C,0x7E09E,0x7031E,0x7061E,0x7C43E,0x7E03E,0x7E03E,0x3EC3C,0x3FE7C,0x1FF78,0x0FFF0,0x07FE0,0x01F80,0x00000},
  {0x00000,0x00000,0x07FE0,0x0FFF0,0x10008,0x30F6C,0x33FEC,0x330CC,0x3606C,0x3606C,0x3606C,0x3606C,0x330CC,0x33FCC,0x30F0C,0x10008,0x0FFF0,0x07FE0,0x00000,0x00000},
};

static void draw_icon(Icon icon, int x, int y, int scale, int app_size = 20) {
  if (icon >= Icon::WHATSAPP && icon <= Icon::INSTAGRAM) {
    const uint32_t *rows = APP_ICON_BITS[(int)icon - (int)Icon::WHATSAPP];
    for (int row = 0; row < app_size; row++) for (int col = 0; col < app_size; col++) {
      int source_row = row * 20 / app_size, source_col = col * 20 / app_size;
      if (rows[source_row] & (1UL << (19 - source_col))) gfx->fillRect(x + col, y + row, 1, 1, SIGN_TEXT_COLOR);
    }
    return;
  }
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

SignContent sign_pick_phrase(int hour, Mood mood) {
  int candidates[PHRASE_COUNT];
  int n = 0;
  auto collect = [&](auto wanted) {
    n = 0;
    for (int i = 0; i < PHRASE_COUNT; i++) {
      const Phrase &p = PHRASES[i];
      bool usable = i != last_phrase && fits_hour(p, hour) &&
                    (p.mood < 0 || p.mood == (int8_t)mood);
      if (usable && wanted(p)) {
        candidates[n++] = i;
      }
    }
  };

  // umore e ora hanno la loro occasione, poi tutte le altre
  if (mood != Mood::NORMALE && random(2) == 0) {
    collect([](const Phrase &p) { return p.mood >= 0; });
  }
  if (n == 0 && random(2) == 0) {
    collect([](const Phrase &p) { return p.from_hour >= 0; });
  }
  if (n == 0) {
    collect([](const Phrase &) { return true; });
  }
  if (n == 0) {
    return {"BOH", Icon::NONE};
  }

  last_phrase = candidates[random(n)];
  const Phrase &p = PHRASES[last_phrase];
  return {p.text, p.icon};
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

struct TextBlock {
  const TextStyle *st;
  char lines[MAX_LINES][MAX_LINE_BYTES];
  int count;
  int16_t ink_x[MAX_LINES];
  uint16_t ink_w[MAX_LINES];
  int ink_top, ink_bottom;
  int height() const { return ink_bottom - ink_top; }
};

// sceglie lo stile piu' grande che ci sta e misura i pixel veri della scritta
static void layout_text(const char *text, int w, int h, TextBlock &b) {
  b.st = &SMALL;
  for (int i = 0; i < STYLE_COUNT; i++) {
    if (wrap(text, *STYLES[i], w - 2 * SIDE_MARGIN, h, b.lines, &b.count)) {
      b.st = STYLES[i];
      break;
    }
  }
  if (b.st == &SMALL) {
    wrap(text, SMALL, w - 2 * SIDE_MARGIN, h, b.lines, &b.count); // anche se non ci sta del tutto
  }
  use_style(*b.st);
  b.ink_top = INT16_MAX;
  b.ink_bottom = INT16_MIN;
  for (int i = 0; i < b.count; i++) {
    int16_t y1;
    uint16_t ink_h;
    gfx->getTextBounds(b.lines[i], 0, 0, &b.ink_x[i], &y1, &b.ink_w[i], &ink_h);
    b.ink_top = min(b.ink_top, i * b.st->line_h + y1);
    b.ink_bottom = max(b.ink_bottom, i * b.st->line_h + y1 + ink_h + b.st->bold_y);
  }
}

// disegna il blocco centrato in orizzontale, con il bordo alto dell'inchiostro a top
static void render_text(const TextBlock &b, int x, int top, int w) {
  use_style(*b.st);
  gfx->setTextColor(SIGN_TEXT_COLOR);
  int first_baseline = top - b.ink_top;
  for (int i = 0; i < b.count; i++) {
    int line_x = x + (w - (b.ink_w[i] + b.st->bold_x)) / 2 - b.ink_x[i];
    int line_y = first_baseline + i * b.st->line_h;
    for (int dx = 0; dx <= b.st->bold_x; dx++) {
      for (int dy = 0; dy <= b.st->bold_y; dy++) {
        gfx->setCursor(line_x + dx, line_y + dy);
        gfx->print(b.lines[i]);
      }
    }
  }
}

static void draw_text(const char *text, int x, int y, int w, int h) {
  TextBlock b;
  layout_text(text, w, h, b);
  render_text(b, x, y + (h - b.height()) / 2, w);
}

void sign_draw(const SignContent &c, int x, int y, int w, int h) {
  if (c.icon == Icon::NONE) {
    draw_text(c.text, x, y, w, h);
    return;
  }
  if ((c.icon >= Icon::WHATSAPP && c.icon <= Icon::INSTAGRAM) && c.text[0] == 0) {
    draw_icon(c.icon, x + (w - 20) / 2, y + (h - 20) / 2, 1);
    return;
  }
  if (c.text[0] == '\0') { // solo l'icona, grande e in mezzo
    int scale = max(1, min(min(w, h) - 8, 40) / 8);
    draw_icon(c.icon, x + (w - 8 * scale) / 2, y + (h - 8 * scale) / 2, scale);
    return;
  }
  int icon_w = 8 * ICON_TEXT_SCALE;
  if (c.icon >= Icon::MESSAGE && c.icon <= Icon::INSTAGRAM) {
    // notifica: icona grande appena sopra il nome, il blocco intero centrato
    const bool app_logo = c.icon >= Icon::WHATSAPP;
    const int icon_size = app_logo ? 20 : 8 * 3;
    const int gap = 2;
    TextBlock b;
    layout_text(c.text, w, h - icon_size - gap, b);
    int top = y + max(0, (h - (icon_size + gap + b.height())) / 2);
    draw_icon(c.icon, x + (w - icon_size) / 2, top, 3);
    render_text(b, x, top + icon_size + gap, w);
    return;
  }
  draw_icon(c.icon, x + SIDE_MARGIN, y + (h - icon_w) / 2, ICON_TEXT_SCALE);
  int shift = icon_w + ICON_GAP; // il margine sinistro del testo resta
  draw_text(c.text, x + shift, y, w - shift, h);
}


void sign_check_phrases(int w, int h) {
  char lines[MAX_LINES][MAX_LINE_BYTES];
  int line_count;
  int icon_shift = 8 * ICON_TEXT_SCALE + ICON_GAP;
  for (int i = 0; i < PHRASE_COUNT; i++) {
    const Phrase &p = PHRASES[i];
    const char *text = p.text;
    int text_w = w - 2 * SIDE_MARGIN - (p.icon != Icon::NONE ? icon_shift : 0);
    if (text[0] && !wrap(text, SMALL, text_w, h, lines, &line_count)) {
      Serial.printf("Frase troppo lunga per il cartoncino: %s\n", text);
    }
  }
}
