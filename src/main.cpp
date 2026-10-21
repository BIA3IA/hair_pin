#include "cartello/cartello.h"
#include "display/display.h"
#include "display/background.h"
#include "imu/imu.h"
#include "rtc/orologio.h"
#include "sprites/sprite.h"
#include "state_machine/state_machine.h"
#include "touch/tocchi_display.h"
#include <Arduino.h>

static const char *current_path = "/Idle/idle_1.bin";

const char *animation_path_for_state(State state) {
  switch (state) {
  case State::WALKING:
    return "/Idle/walking.bin";
  case State::TILTED: {
    const char *options[] = {"/Idle/lose_balance.bin", "/Idle/equilibrio.bin",
                             "/Idle/slide.bin", "/Idle/yanked.bin"};
    return options[random(4)];
  }
  case State::SHAKEN:
    return "/Idle/shake.bin";
  case State::UPSIDE_DOWN:
    return "/Idle/upsidedown.bin";
  case State::RUNNING:
    return "/Idle/running.bin";
  default:
    return "/Idle/idle_1.bin";
  }
}

// quanti frame finali ripetere in loop
// 0 = ripeti tutto
int hold_frames_for_state(State state) {
  switch (state) {
  case State::TILTED:
    return strcmp(current_path, "/Idle/equilibrio.bin") == 0 ? 3 : 5;
  case State::SHAKEN:
    return 3;
  case State::UPSIDE_DOWN:
    return 3;
  default:
    return 0;
  }
}

enum class ExitMode {
  CUT,          // cambia subito
  REVERSE,      // riavvolge fino al frame 0 poi cambia
  FINISH_CYCLE, // finisce il ciclo in corso poi cambia
};

ExitMode exit_mode_for_state(State state) {
  switch (state) {
  case State::WALKING:
  case State::RUNNING:
    return ExitMode::FINISH_CYCLE;
  case State::IDLE:
    return ExitMode::CUT;
  default:
    return ExitMode::REVERSE;
  }
}

static int current_frame = 0;
static int frame_count = 1;
static unsigned long last_frame_time = 0;
const unsigned long FRAME_INTERVAL_MS = 100;
const float SPRITE_SCALE = 1.7;
static int prev_x = 0, prev_y = 0, prev_dw = 0, prev_dh = 0;

const unsigned long HOLD_FRAME_INTERVAL_MS = 180; // più lento su finale
static bool holding = false; // true = avanti-indietro
static int frame_dir = 1;    // +1 avanti, -1 indietro

static State shown_state = State::IDLE;
static bool reversing = false;

enum class PlayMode {
  ONCE, // una volta, poi idle
  LOOP, // ripete il ciclo per un po', poi idle
  HOLD, // ondeggia sugli ultimi frame per un po', poi reverse e idle
  BACKWARD, // una volta, dall'ultimo frame al primo
  SIGN,     // cartello fermo con la frase per un po'
};

struct IdleVariant {
  const char *path;
  PlayMode mode;
  int hold_frames;
  unsigned long min_ms, max_ms; // durata per LOOP e HOLD
  unsigned long frame_ms;       // velocita': ms per frame
};

const IdleVariant IDLE_VARIANTS[] = {
    {"/Idle/steady.bin", PlayMode::ONCE, 0, 0, 0, 160},
    {"/Idle/coffe.bin", PlayMode::ONCE, 0, 0, 0, 200},
    {"/Idle/thoughtful.bin", PlayMode::ONCE, 0, 0, 0, 200},
    {"/Idle/no_head.bin", PlayMode::ONCE, 0, 0, 0, 160},
    {"/Idle/looking_right.bin", PlayMode::ONCE, 0, 0, 0, 160},
    {"/Idle/idle_2.bin", PlayMode::LOOP, 0, 20000, 40000, 100},
    {"/Idle/idle_3.bin", PlayMode::LOOP, 0, 20000, 40000, 100},
    {"/Idle/little_jump_in_place.bin", PlayMode::LOOP, 0, 8000, 15000, 100},
    {"/Idle/bouncing.bin", PlayMode::LOOP, 0, 8000, 15000, 100},
    {"/Idle/phone_typing.bin", PlayMode::HOLD, 2, 5000, 10000, 100},
    {"/Idle/falling_tired.bin", PlayMode::HOLD, 1, 20000, 40000, 100},
    {"/Idle/dance.bin", PlayMode::HOLD, 3, 2000, 4000, 100},
    {"/Idle/strange_pose.bin", PlayMode::HOLD, 3, 3000, 6000, 120},
    {"/Idle/get_phone.bin", PlayMode::HOLD, 3, 3000, 6000, 100},
    {"/Idle/pull_frame_glow.bin", PlayMode::HOLD, 3, 3000, 5000, 100},
    {"/Idle/pull_orb.bin", PlayMode::HOLD, 3, 3000, 5000, 100},
};
const int IDLE_VARIANT_COUNT = sizeof(IDLE_VARIANTS) / sizeof(IDLE_VARIANTS[0]);

// ogni quanto parte una variazione mentre e' in idle
const unsigned long VARIANT_MIN_MS = 12000;
const unsigned long VARIANT_MAX_MS = 45000;

static const IdleVariant *variant = nullptr; // nullptr = nessuna variazione
static unsigned long variant_start = 0;
static unsigned long variant_duration = 0;
static unsigned long variant_timer_start = 0;
static unsigned long variant_delay = 0;

void draw_animation_frame(const char *path) {
  sprite_draw_frame_buffered(path, current_frame, SPRITE_SCALE, prev_x, prev_y, prev_dw,
                             prev_dh, &prev_x, &prev_y, &prev_dw, &prev_dh);
}

void schedule_next_variant() {
  variant_timer_start = millis();
  variant_delay = random(VARIANT_MIN_MS, VARIANT_MAX_MS);
}

void advance_frame(int hold) {
  if (!holding) {
    current_frame++;
    if (current_frame < frame_count) {
      return; // primo giuro
    }
    if (hold <= 0) {
      current_frame = 0;
      return;
    }

    holding = true;
    frame_dir = -1;
    current_frame = frame_count - 1;
  }

  int first = max(0, frame_count - hold); 
  int last = frame_count - 1;             

  current_frame += frame_dir;
  if (current_frame <= first) {
    current_frame = first;
    frame_dir = 1; 
  }
  if (current_frame >= last) {
    current_frame = last;
    frame_dir = -1; 
  }
}

void play(const char *path, bool from_end = false) {
  current_path = path;
  frame_count = sprite_get_frame_count(path);
  current_frame = from_end ? frame_count - 1 : 0;
  holding = false;
  frame_dir = 1;
  reversing = false;
  last_frame_time = millis();
  draw_animation_frame(path);
}

const IdleVariant GET_UP = {"/Idle/falling_tired.bin", PlayMode::BACKWARD, 0, 0, 0, 100};
const IdleVariant PUSH = {"/Idle/push.bin", PlayMode::ONCE, 0, 0, 0, 100};
const IdleVariant BEAT_YOU = {"/Idle/beat_you.bin", PlayMode::HOLD, 3, 1500, 3000, 120};

// cartoncino: lo tira fuori, frase sull'ultimo frame, lo rimette via
const IdleVariant SIGN_UP = {"/Idle/pull_card.bin", PlayMode::ONCE, 0, 0, 0, 100};
const IdleVariant SIGN_HOLD = {"/Idle/pull_card.bin", PlayMode::SIGN, 0, 5000, 8000, 100};
const IdleVariant SIGN_DOWN = {"/Idle/pull_card.bin", PlayMode::BACKWARD, 0, 0, 0, 100};
const int SIGN_CHANCE = 10; // solo dopo il cooldown, fra le variazioni
const unsigned long SIGN_COOLDOWN_MS = 180000;
static unsigned long last_sign_time = 0;
static const char *sign_phrase = nullptr;
static int recent_variants[3] = {-1, -1, -1};

// zona bianca del cartoncino nell'ultimo frame, in pixel della tela originale
const int SIGN_X = 58, SIGN_Y = 63, SIGN_W = 53, SIGN_H = 33; // tutto il cartoncino, da cornice a cornice

static bool reacting = false;

// passi in sequenza (es. spinta -> beat_you); quando finisce l'ultimo torna allo stato
const int MAX_STEPS = 3;
static const IdleVariant *steps[MAX_STEPS] = {};
static int step_index = 0;

void start_animation(State state) {
  shown_state = state;
  variant = nullptr;
  reacting = false;
  steps[0] = steps[1] = steps[2] = nullptr;
  play(animation_path_for_state(state));
  Serial.println("Current state: " + String(static_cast<int>(state)));
  if (state == State::IDLE) {
    schedule_next_variant();
  }
}

void draw_sign_phrase() {
  int ox, oy;
  sprite_canvas_origin(SIGN_HOLD.path, SPRITE_SCALE, &ox, &oy);
  const char *phrase = sign_phrase;
  sign_draw_text(phrase, ox + (int)(SIGN_X * SPRITE_SCALE),
                 oy + (int)(SIGN_Y * SPRITE_SCALE), (int)(SIGN_W * SPRITE_SCALE),
                 (int)(SIGN_H * SPRITE_SCALE));
}

void start_variant_step(const IdleVariant *v) {
  variant = v;
  variant_start = millis();
  variant_duration = random(v->min_ms, v->max_ms);
  play(v->path,
       v->mode == PlayMode::BACKWARD || v->mode == PlayMode::SIGN);
  if (v->mode == PlayMode::SIGN) {
    sign_phrase = sign_pick_phrase(rtc_hour());
    last_sign_time = millis();
    draw_sign_phrase();
  }
}

void start_steps(const IdleVariant *a, const IdleVariant *b = nullptr,
                 const IdleVariant *c = nullptr) {
  steps[0] = a;
  steps[1] = b;
  steps[2] = c;
  step_index = 0;
  start_variant_step(a);
}

// false quando non ci sono altri passi
bool next_step() {
  step_index++;
  if (step_index >= MAX_STEPS || steps[step_index] == nullptr) {
    return false;
  }
  start_variant_step(steps[step_index]);
  return true;
}

void start_variant() {
  if (millis() - last_sign_time >= SIGN_COOLDOWN_MS &&
      random(100) < SIGN_CHANCE) {
    start_steps(&SIGN_UP, &SIGN_HOLD, &SIGN_DOWN);
    return;
  }
  int candidates[IDLE_VARIANT_COUNT];
  int count = 0;
  for (int i = 0; i < IDLE_VARIANT_COUNT; i++) {
    if (i != recent_variants[0] && i != recent_variants[1] &&
        i != recent_variants[2]) {
      candidates[count++] = i;
    }
  }
  int chosen = candidates[random(count)];
  recent_variants[2] = recent_variants[1];
  recent_variants[1] = recent_variants[0];
  recent_variants[0] = chosen;
  start_steps(&IDLE_VARIANTS[chosen]);
  Serial.printf("Variazione idle: %s\n", variant->path);
}

void start_reaction(bool from_ground = false) {
  reacting = true;
  shown_state = State::ANNOYED;
  start_steps(from_ground ? &GET_UP : &PUSH, &BEAT_YOU);
  Serial.println("Toccata!");
}

// false quando la variazione e' finita
bool advance_variant() {
  bool time_up = millis() - variant_start >= variant_duration;

  switch (variant->mode) {
  case PlayMode::ONCE:
    current_frame++;
    return current_frame < frame_count;

  case PlayMode::LOOP:
    current_frame++;
    if (current_frame < frame_count) {
      return true;
    }
    current_frame = 0;
    return !time_up;

  case PlayMode::HOLD:
    if (reversing) {
      current_frame--;
      return current_frame >= 0;
    }
    advance_frame(variant->hold_frames);
    if (holding && time_up) {
      reversing = true;
      holding = false;
    }
    return true;

  case PlayMode::BACKWARD:
    current_frame--;
    return current_frame >= 0;

  case PlayMode::SIGN:
    return !time_up;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  display_init();
  imu_init();
  touch_init();
  rtc_init();
  sprite_init();
  sign_check_phrases((int)(SIGN_W * SPRITE_SCALE), (int)(SIGN_H * SPRITE_SCALE));
  start_animation(State::IDLE);
  delay(1000);
  Serial.println("Bianca clip: ciao mondo, firmware vivo!");
}

void loop() {
  background_update(prev_x, prev_y, prev_dw, prev_dh);
  state_machine_update(imu_update());
  State target = state_machine_current();

  // la spinta parte solo se e' in piedi tranquilla o sta camminando
  // (non durante le HOLD, a terra, al telefono..., ne' col cartello in mano)
  bool calm_variant = variant != nullptr && steps[1] == nullptr &&
                      (variant->mode == PlayMode::ONCE ||
                       variant->mode == PlayMode::LOOP);
  bool calm_idle =
      shown_state == State::IDLE && (variant == nullptr || calm_variant);
  bool can_be_pushed = !reacting && !reversing &&
                       (calm_idle || shown_state == State::WALKING);

  bool can_get_up = !reacting && !reversing && holding &&
                    variant != nullptr &&
                    strcmp(variant->path, GET_UP.path) == 0;
  if (touch_tapped()) {
    if (can_get_up) {
      start_reaction(true); // prima si rialza, poi si arrabbia
    } else if (can_be_pushed) {
      start_reaction();
    }
  }

  if (reacting) {
    // niente, aspetta fine
  } else if (target != shown_state) {
    ExitMode mode = exit_mode_for_state(shown_state);
    if (mode == ExitMode::CUT || current_frame == 0) {
      start_animation(target);
    } else if (mode == ExitMode::REVERSE) {
      reversing = true;
      holding = false;
    }
  } else if (variant == nullptr) {
    if (reversing) {
      reversing = false;
      frame_dir = 1;
    } else if (shown_state == State::IDLE &&
               millis() - variant_timer_start >= variant_delay) {
      start_variant();
    }
  }

  unsigned long interval = holding             ? HOLD_FRAME_INTERVAL_MS
                           : variant != nullptr ? variant->frame_ms
                                                : FRAME_INTERVAL_MS;

  if (millis() - last_frame_time >= interval) {
    if (variant != nullptr) {
      if (!advance_variant()) {
        if (!next_step()) {
          start_animation(target);
        }
        return;
      }
    } else if (reversing) {
      current_frame--;
    } else {
      advance_frame(hold_frames_for_state(shown_state));
    }
    last_frame_time = millis();
  }

  static unsigned long last_render_time = 0;
  if (millis() - last_render_time >= 33) {
    last_render_time = millis();
    // La lavagna ferma e' gia' completa di testo: ridisegnare il frame
    // cancellerebbe la frase prima di riscriverla, causando sfarfallio.
    // background_update continua ad animare la zona esterna alla sprite.
    if (variant == nullptr || variant->mode != PlayMode::SIGN) {
      draw_animation_frame(current_path);
    }
  }

  delay(1);
}
