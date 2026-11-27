#include "comportamento.h"
#include "../cartello/cartello.h"
#include "../display/display.h"
#include "../player/player.h"
#include "../sprites/sprite.h"
#include "../ble/protocollo.h"
#include "../batteria/batteria.h"
#include <Preferences.h>

namespace {
// le durate delle HOLD contano dall'inizio, compresa la parte in avanti

const Clip IDLE_BASE = {"/Idle/idle_1.bin", PlayMode::LOOP, 0, 0, 0, 100};
const Clip WALKING = {"/Idle/walking.bin", PlayMode::LOOP, 0, 0, 0, 100};
const Clip RUNNING = {"/Idle/running.bin", PlayMode::LOOP, 0, 0, 0, 100};
const Clip SHAKEN = {"/Idle/shake.bin", PlayMode::HOLD, 3, 0, 0, 100};
const Clip UPSIDE_DOWN = {"/Idle/upsidedown.bin", PlayMode::HOLD, 3, 0, 0, 100};
const Clip TILTED[] = {
    {"/Idle/lose_balance.bin", PlayMode::HOLD, 5, 0, 0, 100},
    {"/Idle/equilibrio.bin", PlayMode::LOOP, 0, 0, 0, 100},
    {"/Idle/slide.bin", PlayMode::HOLD, 5, 0, 0, 100},
    {"/Idle/yanked.bin", PlayMode::HOLD, 5, 0, 0, 100},
};

const Clip VARIANTS[] = {
    {"/Idle/steady.bin", PlayMode::FORWARD, 0, 0, 0, 160},
    {"/Idle/coffe.bin", PlayMode::FORWARD, 0, 0, 0, 200},
    {"/Idle/thoughtful.bin", PlayMode::FORWARD, 0, 0, 0, 200},
    {"/Idle/no_head.bin", PlayMode::FORWARD, 0, 0, 0, 160},
    {"/Idle/looking_right.bin", PlayMode::FORWARD, 0, 0, 0, 160},
    {"/Idle/idle_2.bin", PlayMode::LOOP, 0, 20000, 40000, 100},
    {"/Idle/idle_3.bin", PlayMode::LOOP, 0, 20000, 40000, 100},
    {"/Idle/little_jump_in_place.bin", PlayMode::LOOP, 0, 25000, 45000, 100},
    {"/Idle/bouncing.bin", PlayMode::LOOP, 0, 8000, 15000, 100},
    {"/Idle/falling_tired.bin", PlayMode::HOLD, 1, 20000, 40000, 100},
    {"/Idle/dance.bin", PlayMode::HOLD, 3, 2000, 4000, 100},
    {"/Idle/strange_pose.bin", PlayMode::FORWARD, 0, 0, 0, 120},
    {"/Idle/get_phone.bin", PlayMode::HOLD, 3, 3000, 6000, 100},
    {"/Idle/pull_frame_glow.bin", PlayMode::HOLD, 3, 3000, 5000, 100},
    {"/Idle/pull_orb.bin", PlayMode::HOLD, 3, 3000, 5000, 100},
    {"/Idle/wave.bin", PlayMode::FORWARD, 0, 0, 0, 90},
    {"/Idle/sad.bin", PlayMode::HOLD, 4, 7000, 12000, 120},
    {"/Idle/grumpy.bin", PlayMode::HOLD, 6, 6000, 10000, 110},
};
const int VARIANT_COUNT = sizeof(VARIANTS) / sizeof(VARIANTS[0]);

const Clip SIGN = {"/Idle/pull_card.bin", PlayMode::SIGN, 0, 6700, 9700, 100};
const Clip BATTERY_SIGN = SIGN;
const Clip SLEEP = {"/Idle/falling_asleep.bin", PlayMode::HOLD, 3, 1800000, 3600000, 160};

const Clip PUSH = {"/Idle/push.bin", PlayMode::FORWARD, 0, 0, 0, 100};
const Clip BEAT_YOU = {"/Idle/beat_you.bin", PlayMode::HOLD, 3, 2800, 4300, 120};
const Clip GRUMPY = {"/Idle/grumpy.bin", PlayMode::HOLD, 6, 4000, 6000, 110};
const Clip FED_UP = {"/Idle/grumpy.bin", PlayMode::HOLD, 6, 8000, 11000, 110}; // basta tocchi
const Clip CUDDLED = {"/Idle/cuddled.bin", PlayMode::HOLD, 4, 4500, 6000, 110};
const Clip WAVE = {"/Idle/wave.bin", PlayMode::FORWARD, 0, 0, 0, 90};
const Clip STARTLED = {"/Idle/startled.bin", PlayMode::HOLD, 3, 1800, 2500, 70};
const Clip GLANCE_PHONE = {"/Idle/get_phone.bin", PlayMode::ONCE, 0, 0, 0, 90};
const Clip MESSAGE_SIGN = {"/Idle/pull_card.bin", PlayMode::SIGN, 0, 6500, 8000, 100};
const Clip PASSKEY_SIGN = {"/Idle/pull_card.bin", PlayMode::SIGN, 0, 60000, 60000, 100};

const unsigned long VARIANT_MIN_MS = 12000;
const unsigned long VARIANT_MAX_MS = 45000;
const int SIGN_CHANCE = 10; // %, solo dopo il cooldown
const unsigned long SIGN_COOLDOWN_MS = 180000;
const unsigned long TAP_WINDOW_MS = 20000;
const unsigned long LONG_ABSENCE_MS = 30UL * 60 * 1000;
const uint8_t SLEEP_BRIGHTNESS = 20;
const unsigned long MESSAGE_WAIT_MS = 60000; // se e' impegnata aspetta al massimo cosi'

// zona bianca del cartoncino nell'ultimo frame, in pixel della tela originale
const int SIGN_X = 58, SIGN_Y = 63, SIGN_W = 53, SIGN_H = 33;

const Clip REMOTE_CLIPS[] = {
  {"/Idle/beat_you.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/bouncing.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/coffe.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/cuddled.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/dance.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/equilibrio.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/falling_asleep.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/falling_tired.bin", PlayMode::HOLD, 1, 20000, 40000, 100},
  {"/Idle/get_phone.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/grumpy.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/idle_1.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/idle_2.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/idle_3.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/little_jump_in_place.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/looking_right.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/lose_balance.bin", PlayMode::HOLD, 5, 4000, 6000, 100},
  {"/Idle/no_head.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/pull_card.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/pull_frame_glow.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/pull_orb.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/push.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/running.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/sad.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/shake.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/slide.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/startled.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/steady.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/strange_pose.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/thoughtful.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/upsidedown.bin", PlayMode::ONCE, 0, 0, 0, 100},
  {"/Idle/walking.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/wave.bin", PlayMode::FORWARD, 0, 0, 0, 100},
  {"/Idle/yanked.bin", PlayMode::ONCE, 0, 0, 0, 100},
};
const Clip *remote_pending = nullptr;
// frasi dall'app: si prepara la meta' libera e si scambia solo al commit
constexpr uint8_t PHRASE_MAX = 64;
char phrase_slots[2][PHRASE_MAX][49] = {};
uint8_t active_slot = 0, phrase_count = 0, staging_count = 0;
bool staging_valid = false, manual_sign = false;
State shown = State::IDLE;
bool reacting = false; // reazione in corso: lo stato aspetta
bool leaving = false;  // sta chiudendo l'animazione per cambiare stato
const Clip *queue[3] = {};
int queue_len = 0, queue_pos = 0;
int hour = -1;

unsigned long variant_timer = 0, variant_delay = 0;
unsigned long last_sign_time = 0;
unsigned long last_battery_sign = 0, battery_sign_delay = 0;
int recent[3] = {-1, -1, -1};

char message_text[64] = "";
Icon message_icon = Icon::NONE;
bool message_pending = false;
unsigned long message_time = 0;
char passkey_text[8] = "";

uint8_t brightness = 255; // scelta dall'app, resta dopo i riavvii

int tap_count = 0;
unsigned long last_tap_time = 0, last_touch_time = 0;

bool is_night() { return hour >= 0 && (hour >= 23 || hour < 7); }

bool is_variant(const Clip *c) {
  return c >= VARIANTS && c < VARIANTS + VARIANT_COUNT;
}

void schedule_variant() {
  variant_timer = millis();
  variant_delay = is_night() ? random(4000, 8000) // si riaddormenta presto
                             : random(VARIANT_MIN_MS, VARIANT_MAX_MS);
}

const Clip *clip_for(State state) {
  switch (state) {
  case State::WALKING:
    return &WALKING;
  case State::RUNNING:
    return &RUNNING;
  case State::TILTED:
    return &TILTED[random(4)];
  case State::SHAKEN:
    return &SHAKEN;
  case State::UPSIDE_DOWN:
    return &UPSIDE_DOWN;
  default:
    return &IDLE_BASE;
  }
}

void start_state(State state) {
  shown = state;
  reacting = leaving = false;
  queue_len = queue_pos = 0;
  player_start(clip_for(state));
  Serial.printf("Stato %d\n", (int)state);
  if (state == State::IDLE) {
    schedule_variant();
  }
}

bool play_next() {
  if (queue_pos >= queue_len) {
    return false;
  }
  player_start(queue[queue_pos++]);
  return true;
}

// mette in coda le clip e chiude con garbo quella in corso
void play(const Clip *a, const Clip *b = nullptr, const Clip *c = nullptr,
          bool reaction = false) {
  queue_len = queue_pos = 0;
  for (const Clip *clip : {a, b, c}) {
    if (clip != nullptr) {
      queue[queue_len++] = clip;
    }
  }
  if (reaction) {
    reacting = true;
    shown = State::ANNOYED;
  }
  const Clip *current = player_clip();
  bool walking = current == &WALKING || current == &RUNNING;
  if ((current == &IDLE_BASE && (reaction || player_frame() == 0)) ||
      (walking && reaction)) {
    play_next(); // dalla posa base (o camminando) si parte subito
  } else {
    player_stop(reaction ? StopStyle::QUICK : StopStyle::GENTLE);
  }
}

void start_variant() {
  if (is_night()) {
    play(&SLEEP);
    return;
  }
  if (batteria_percentuale() <= 100 && millis() - last_battery_sign >= battery_sign_delay &&
      millis() - last_sign_time >= SIGN_COOLDOWN_MS) {
    play(&BATTERY_SIGN);
    return;
  }
  if (millis() - last_sign_time >= SIGN_COOLDOWN_MS && random(100) < SIGN_CHANCE) {
    play(&SIGN);
    return;
  }

  // a caso, ma non una delle ultime tre
  int candidates[VARIANT_COUNT];
  int count = 0;
  for (int i = 0; i < VARIANT_COUNT; i++) {
    if (i != recent[0] && i != recent[1] && i != recent[2]) candidates[count++] = i;
  }
  int chosen = candidates[random(count)];
  recent[2] = recent[1];
  recent[1] = recent[0];
  recent[0] = chosen;
  bool double_glance = strcmp(VARIANTS[chosen].path, "/Idle/looking_right.bin") == 0 && random(4) == 0;
  play(&VARIANTS[chosen], double_glance ? &VARIANTS[chosen] : nullptr);
  Serial.printf("Variazione: %s\n", VARIANTS[chosen].path);
}

void draw_sign() {
  int ox, oy;
  sprite_canvas_origin(SIGN.path, &ox, &oy);
  const Clip *clip = player_clip();
  SignContent content;
  char battery_text[8];
  if (clip == &PASSKEY_SIGN) {
    content = {passkey_text, Icon::NONE};
  } else if (clip == &MESSAGE_SIGN) {
    content = {message_text, message_icon};
  } else if (clip == &BATTERY_SIGN && batteria_percentuale() <= 100) {
    snprintf(battery_text, sizeof(battery_text), "%u%%", batteria_percentuale());
    content = {battery_text, Icon::NONE};
    last_sign_time = last_battery_sign = millis();
    battery_sign_delay = random(10UL * 60000, 20UL * 60000);
  } else {
    content = phrase_count ? SignContent{phrase_slots[active_slot][random(phrase_count)], Icon::NONE}
                           : sign_pick_phrase(hour);
    last_sign_time = millis(); // conta solo se il cartello si e' visto davvero
  }
  sprite_invalidate();
  sign_draw(content, ox + (int)(SIGN_X * SPRITE_SCALE), oy + (int)(SIGN_Y * SPRITE_SCALE),
            (int)(SIGN_W * SPRITE_SCALE), (int)(SIGN_H * SPRITE_SCALE));
}

bool busy() { return reacting || leaving || queue_pos < queue_len; }

// in piedi tranquilla o mentre cammina: non durante le HOLD, a terra,
// al telefono o col cartello
bool calm() {
  if (reacting || leaving) {
    return false;
  }
  const Clip *c = player_clip();
  if (shown == State::WALKING || c == &IDLE_BASE) {
    return true;
  }
  return shown == State::IDLE && !busy() && !player_stopping() && is_variant(c) &&
         (c->mode == PlayMode::ONCE || c->mode == PlayMode::FORWARD || c->mode == PlayMode::LOOP);
}

// si puo' interrompere per reagire: si sveglia, si rialza, e per le coccole
// anche quando e' triste o imbronciata
bool comforting(const Clip *c) {
  return c->mode == PlayMode::HOLD &&
         (strstr(c->path, "/sad.bin") || strstr(c->path, "/grumpy.bin") || strstr(c->path, "/falling_tired.bin"));
}

bool interruptible(bool cuddle) {
  const Clip *c = player_clip();
  if (busy() || shown != State::IDLE || player_stopping()) {
    return false;
  }
  if (c == &SLEEP) {
    return true;
  }
  if (strcmp(c->path, "/Idle/falling_tired.bin") == 0) {
    return player_holding();
  }
  return cuddle && comforting(c);
}

void handle_touch(TouchEvent touch) {
  bool cuddle = touch == TouchEvent::LONG_PRESS;
  if (!calm() && !interruptible(cuddle)) {
    return;
  }
  unsigned long now = millis();
  bool long_absence = now - last_touch_time >= LONG_ABSENCE_MS;
  bool was_sleeping = player_clip() == &SLEEP;
  last_touch_time = now;

  if (cuddle) {
    tap_count = 0;
    play(&CUDDLED, nullptr, nullptr, true);
    Serial.println("Coccole");
    return;
  }

  if (now - last_tap_time > TAP_WINDOW_MS) {
    tap_count = 0;
  }
  tap_count++;
  last_tap_time = now;

  if (was_sleeping) {
    play(&GRUMPY, nullptr, nullptr, true);
  } else if (tap_count >= 5) {
    tap_count = 0;
    play(&PUSH, &FED_UP, nullptr, true);
  } else if (tap_count >= 3) {
    play(&PUSH, &GRUMPY, nullptr, true);
  } else if (long_absence) {
    play(&WAVE, nullptr, nullptr, true);
  } else {
    play(&PUSH, &BEAT_YOU, nullptr, true);
  }
  Serial.printf("Tocco %d\n", tap_count);
}
} // namespace

void comportamento_init(int current_hour) {
  hour = current_hour;
  Preferences prefs;
  prefs.begin("bianca", true);
  brightness = max<uint8_t>(prefs.getUChar("luce", 255), MIN_BRIGHTNESS);
  prefs.end();
  last_battery_sign = millis();
  battery_sign_delay = random(10UL * 60000, 20UL * 60000);
  sign_check_phrases((int)(SIGN_W * SPRITE_SCALE), (int)(SIGN_H * SPRITE_SCALE));
  start_state(State::IDLE);
  last_touch_time = millis();
  play(&WAVE); // ciao!
}

void comportamento_update(State target, TouchEvent touch, bool jolt,
                          int current_hour, bool background_moved) {
  hour = current_hour;

  if (touch != TouchEvent::NONE) {
    handle_touch(touch);
  }

  bool can_startle = shown == State::IDLE && (calm() || interruptible(false));
  if (jolt && can_startle) {
    play(&STARTLED, nullptr, nullptr, true);
    Serial.println("Spavento");
  }

  if (!reacting && !leaving && target != shown) {
    leaving = true;
    queue_len = queue_pos = 0;
    const Clip *current = player_clip();
    if (current == &IDLE_BASE) {
      start_state(target);
    } else {
      player_stop(current == &WALKING || current == &RUNNING ? StopStyle::GENTLE
                                                              : StopStyle::QUICK);
    }
  }

  if (remote_pending && !busy()) {
    const Clip *chosen = remote_pending;
    remote_pending = nullptr;
    play(chosen, nullptr, nullptr, true);
  }

  if (message_pending) {
    bool can_show = (shown == State::IDLE || shown == State::WALKING) &&
                    (calm() || interruptible(false));
    if (millis() - message_time > MESSAGE_WAIT_MS) {
      message_pending = false;
    } else if (can_show) {
      message_pending = false;
      if (manual_sign) play(&MESSAGE_SIGN, nullptr, nullptr, true);
      else play(&GLANCE_PHONE, &MESSAGE_SIGN, nullptr, true);
      manual_sign = false;
    }
  }

  if (shown == State::IDLE && !busy() && player_clip() == &IDLE_BASE &&
      millis() - variant_timer >= variant_delay) {
    start_variant();
  }

  switch (player_update(background_moved)) {
  case PlayerEvent::SIGN_READY:
    draw_sign();
    break;
  case PlayerEvent::DONE:
    if (!play_next()) {
      start_state(target);
    }
    break;
  case PlayerEvent::NONE:
    break;
  }

  bool asleep = player_clip() == &SLEEP && player_holding();
  display_set_brightness(asleep ? min(SLEEP_BRIGHTNESS, brightness) : brightness);
}

void comportamento_notify(Icon icon, const char *text) {
  manual_sign = false;
  strlcpy(message_text, text, sizeof(message_text));
  message_icon = icon;
  message_time = millis();
  message_pending = true;
  Serial.println("Notifica in arrivo");
}

void comportamento_show_passkey(uint32_t code) {
  snprintf(passkey_text, sizeof(passkey_text), "%06lu", (unsigned long)code);
  play(&PASSKEY_SIGN, nullptr, nullptr, true);
}

void comportamento_pairing_done() {
  if (player_clip() == &PASSKEY_SIGN) {
    player_stop(StopStyle::GENTLE);
  }
}

void comportamento_set_brightness(uint8_t level) {
  if (level == brightness) return;
  brightness = level;
  Preferences prefs;
  prefs.begin("bianca", false);
  prefs.putUChar("luce", level);
  prefs.end();
  Serial.printf("Luminosita' %u\n", level);
}

uint8_t comportamento_brightness() { return brightness; }

bool comportamento_sleeping() { return player_clip() == &SLEEP; }

void comportamento_clear_phrases() {
  phrase_count = staging_count = 0;
  staging_valid = false;
}

void comportamento_command(uint8_t action, const char *text) {
  if (action == 1) {
    if (!valid_animation(text)) return;
    char path[64];
    snprintf(path, sizeof(path), "/Idle/%s.bin", text);
    for (const Clip &clip : REMOTE_CLIPS) {
      if (strcmp(clip.path, path) == 0) { remote_pending = &clip; break; }
    }
  } else if (action == 2) {
    strlcpy(message_text, text, sizeof(message_text));
    message_icon = Icon::NONE;
    message_time = millis();
    message_pending = manual_sign = true;
  } else if (action == 3) {
    staging_count = 0;
    staging_valid = true;
  } else if (action == 4 && staging_valid) {
    if (staging_count == PHRASE_MAX) { staging_valid = false; return; }
    strlcpy(phrase_slots[active_slot ^ 1][staging_count++], text, 49);
  } else if (action == 5 && staging_valid) {
    active_slot ^= 1;
    phrase_count = staging_count;
    staging_valid = false;
  }
}
