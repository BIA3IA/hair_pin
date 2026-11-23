#include "comportamento.h"
#include "../cartello/cartello.h"
#include "../display/display.h"
#include "../player/player.h"
#include "../sprites/sprite.h"
#include "../umore/umore.h"

namespace {
// le durate delle HOLD contano dall'inizio, compresa la parte in avanti

const Clip IDLE_BASE = {"/Idle/idle_1.bin", PlayMode::LOOP, 0, 0, 0, 100};
const Clip WALKING = {"/Idle/walking.bin", PlayMode::LOOP, 0, 0, 0, 100};
const Clip RUNNING = {"/Idle/running.bin", PlayMode::LOOP, 0, 0, 0, 100};
const Clip SHAKEN = {"/Idle/shake.bin", PlayMode::HOLD, 3, 0, 0, 100};
const Clip UPSIDE_DOWN = {"/Idle/upsidedown.bin", PlayMode::HOLD, 3, 0, 0, 100};
const Clip TILTED[] = {
    {"/Idle/lose_balance.bin", PlayMode::HOLD, 5, 0, 0, 100},
    {"/Idle/equilibrio.bin", PlayMode::HOLD, 3, 0, 0, 100},
    {"/Idle/slide.bin", PlayMode::HOLD, 5, 0, 0, 100},
    {"/Idle/yanked.bin", PlayMode::HOLD, 5, 0, 0, 100},
};

const Clip VARIANTS[] = {
    {"/Idle/steady.bin", PlayMode::ONCE, 0, 0, 0, 160},
    {"/Idle/coffe.bin", PlayMode::ONCE, 0, 0, 0, 200},
    {"/Idle/thoughtful.bin", PlayMode::ONCE, 0, 0, 0, 200},
    {"/Idle/no_head.bin", PlayMode::ONCE, 0, 0, 0, 160},
    {"/Idle/looking_right.bin", PlayMode::ONCE, 0, 0, 0, 160},
    {"/Idle/idle_2.bin", PlayMode::LOOP, 0, 20000, 40000, 100},
    {"/Idle/idle_3.bin", PlayMode::LOOP, 0, 20000, 40000, 100},
    {"/Idle/little_jump_in_place.bin", PlayMode::LOOP, 0, 8000, 15000, 100, Tone::HAPPY},
    {"/Idle/bouncing.bin", PlayMode::LOOP, 0, 8000, 15000, 100, Tone::HAPPY},
    {"/Idle/falling_tired.bin", PlayMode::HOLD, 1, 20000, 40000, 100, Tone::GLOOMY},
    {"/Idle/dance.bin", PlayMode::HOLD, 3, 2000, 4000, 100, Tone::HAPPY},
    {"/Idle/strange_pose.bin", PlayMode::HOLD, 3, 3000, 6000, 120},
    {"/Idle/get_phone.bin", PlayMode::HOLD, 3, 3000, 6000, 100},
    {"/Idle/pull_frame_glow.bin", PlayMode::HOLD, 3, 3000, 5000, 100, Tone::HAPPY},
    {"/Idle/pull_orb.bin", PlayMode::HOLD, 3, 3000, 5000, 100, Tone::HAPPY},
    {"/Idle/wave.bin", PlayMode::ONCE, 0, 0, 0, 90, Tone::HAPPY},
    {"/Idle/sad.bin", PlayMode::HOLD, 4, 7000, 12000, 120, Tone::GLOOMY},
    {"/Idle/grumpy.bin", PlayMode::HOLD, 6, 6000, 10000, 110, Tone::GLOOMY},
};
const int VARIANT_COUNT = sizeof(VARIANTS) / sizeof(VARIANTS[0]);

const Clip SIGN = {"/Idle/pull_card.bin", PlayMode::SIGN, 0, 6700, 9700, 100};
const Clip SLEEP = {"/Idle/falling_asleep.bin", PlayMode::HOLD, 3, 1800000, 3600000, 160};

const Clip PUSH = {"/Idle/push.bin", PlayMode::FORWARD, 0, 0, 0, 100};
const Clip BEAT_YOU = {"/Idle/beat_you.bin", PlayMode::HOLD, 3, 2800, 4300, 120};
const Clip HAPPY_HOP = {"/Idle/little_jump_in_place.bin", PlayMode::ONCE, 0, 0, 0, 90};
const Clip GRUMPY = {"/Idle/grumpy.bin", PlayMode::HOLD, 6, 4000, 6000, 110};
const Clip TURN_AWAY = {"/Idle/turn_away.bin", PlayMode::HOLD, 1, 6500, 9500, 90};
const Clip CUDDLED = {"/Idle/cuddled.bin", PlayMode::HOLD, 4, 4500, 6000, 110};
const Clip WAVE = {"/Idle/wave.bin", PlayMode::ONCE, 0, 0, 0, 90};
const Clip STARTLED = {"/Idle/startled.bin", PlayMode::HOLD, 3, 1800, 2500, 70};

const unsigned long VARIANT_MIN_MS = 12000;
const unsigned long VARIANT_MAX_MS = 45000;
const int SIGN_CHANCE = 10; // %, solo dopo il cooldown
const unsigned long SIGN_COOLDOWN_MS = 180000;
const unsigned long TAP_WINDOW_MS = 20000;
const unsigned long LONG_ABSENCE_MS = 30UL * 60 * 1000;
const uint8_t SLEEP_BRIGHTNESS = 20;

// zona bianca del cartoncino nell'ultimo frame, in pixel della tela originale
const int SIGN_X = 58, SIGN_Y = 63, SIGN_W = 53, SIGN_H = 33;

State shown = State::IDLE;
bool reacting = false; // reazione in corso: lo stato aspetta
bool leaving = false;  // sta chiudendo l'animazione per cambiare stato
const Clip *queue[3] = {};
int queue_len = 0, queue_pos = 0;
int hour = -1;

unsigned long variant_timer = 0, variant_delay = 0;
unsigned long last_sign_time = 0;
int recent[3] = {-1, -1, -1};

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
  if (state == State::SHAKEN) {
    umore_add(-3);
  } else if (state == State::UPSIDE_DOWN) {
    umore_add(-2);
  }
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

// peso di ogni tono secondo l'umore
int tone_weight(Tone tone) {
  //                          CALM HAPPY GLOOMY
  static const int W[4][3] = {{2, 0, 5},  // triste
                              {3, 1, 3},  // giu'
                              {3, 2, 1},  // normale
                              {2, 5, 0}}; // felice
  return W[(int)umore_level()][(int)tone];
}

void start_variant() {
  if (is_night()) {
    play(&SLEEP);
    return;
  }
  if (millis() - last_sign_time >= SIGN_COOLDOWN_MS && random(100) < SIGN_CHANCE) {
    play(&SIGN);
    return;
  }

  int candidates[VARIANT_COUNT], weights[VARIANT_COUNT];
  int count = 0, total = 0;
  for (int i = 0; i < VARIANT_COUNT; i++) {
    int w = tone_weight(VARIANTS[i].tone);
    if (w > 0 && i != recent[0] && i != recent[1] && i != recent[2]) {
      candidates[count] = i;
      weights[count++] = w;
      total += w;
    }
  }
  if (count == 0) {
    schedule_variant();
    return;
  }
  int pick = random(total), chosen = candidates[count - 1];
  for (int i = 0; i < count; i++) {
    if (pick < weights[i]) {
      chosen = candidates[i];
      break;
    }
    pick -= weights[i];
  }
  recent[2] = recent[1];
  recent[1] = recent[0];
  recent[0] = chosen;
  play(&VARIANTS[chosen]);
  Serial.printf("Variazione: %s\n", VARIANTS[chosen].path);
}

void draw_sign() {
  int ox, oy;
  sprite_canvas_origin(SIGN.path, &ox, &oy);
  SignContent content = sign_pick_phrase(hour, umore_level(), umore_steps_today());
  last_sign_time = millis(); // conta solo se il cartello si e' visto davvero
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
         (c->mode == PlayMode::ONCE || c->mode == PlayMode::LOOP);
}

// si puo' interrompere per reagire: si sveglia, si rialza, e per le coccole
// anche quando e' triste o imbronciata
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
  return cuddle && c->tone == Tone::GLOOMY && c->mode == PlayMode::HOLD;
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
  umore_attention();

  if (cuddle) {
    tap_count = 0;
    umore_add(8);
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
    umore_add(-1);
    play(&GRUMPY, nullptr, nullptr, true);
  } else if (tap_count >= 5) {
    tap_count = 0;
    umore_add(-8);
    play(&TURN_AWAY, nullptr, nullptr, true);
  } else if (tap_count >= 3) {
    umore_add(-3);
    play(&PUSH, &GRUMPY, nullptr, true);
  } else if (long_absence) {
    umore_add(2);
    play(&WAVE, nullptr, nullptr, true);
  } else if (umore_level() == Mood::FELICE) {
    umore_add(1);
    play(&HAPPY_HOP, nullptr, nullptr, true);
  } else {
    play(&PUSH, &BEAT_YOU, nullptr, true);
  }
  Serial.printf("Tocco %d\n", tap_count);
}
} // namespace

void comportamento_init(int current_hour) {
  hour = current_hour;
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
    umore_add(-1);
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
  display_set_brightness(asleep ? SLEEP_BRIGHTNESS : 255);
}
