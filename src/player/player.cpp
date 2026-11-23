#include "player.h"
#include "../sprites/sprite.h"

namespace {
const unsigned long HOLD_FRAME_MS = 180;
const unsigned long QUICK_RETURN_MS = 50;
const unsigned long GENTLE_RETURN_MAX_MS = 100;

enum class Phase { PLAY, HOLD, RETURN, DONE };

const Clip *clip = nullptr;
Phase phase = Phase::DONE;
int frame = 0, frame_count = 1, hold_dir = -1;
unsigned long started = 0, duration = 0, last_step = 0;
bool stop_requested = false;
StopStyle stop_style = StopStyle::GENTLE;

int area_x = 0, area_y = 0, area_w = 0, area_h = 0;
const char *drawn_path = nullptr;
int drawn_frame = -1;

void draw() {
  sprite_draw_frame_buffered(clip->path, frame, area_x, area_y, area_w, area_h,
                             &area_x, &area_y, &area_w, &area_h);
  drawn_path = clip->path;
  drawn_frame = frame;
}

unsigned long step_interval() {
  bool quick = stop_requested && stop_style == StopStyle::QUICK;
  switch (phase) {
  case Phase::HOLD:
    return clip->mode == PlayMode::FORWARD ? clip->frame_ms : HOLD_FRAME_MS;
  case Phase::RETURN:
    if (quick) {
      return QUICK_RETURN_MS;
    }
    // il frame finale resta quanto gli altri, poi torna piu' svelta
    return frame == frame_count - 1
               ? clip->frame_ms
               : min((unsigned long)clip->frame_ms, GENTLE_RETURN_MAX_MS);
  default:
    return clip->frame_ms;
  }
}

PlayerEvent reach_last_frame() {
  switch (clip->mode) {
  case PlayMode::FORWARD: // l'ultimo frame si vede, poi finisce
  case PlayMode::HOLD:
    phase = Phase::HOLD;
    hold_dir = -1;
    return PlayerEvent::NONE;
  case PlayMode::SIGN:
    phase = Phase::HOLD;
    return PlayerEvent::SIGN_READY;
  default:
    phase = Phase::RETURN;
    return PlayerEvent::NONE;
  }
}

PlayerEvent step() {
  unsigned long now = millis();
  bool time_up = duration > 0 && now - started >= duration;
  int last = frame_count - 1;

  switch (phase) {
  case Phase::PLAY:
    if (clip->mode == PlayMode::BACKWARD) {
      if (frame == 0) {
        phase = Phase::DONE;
        return PlayerEvent::DONE;
      }
      frame--;
      return PlayerEvent::NONE;
    }
    if (clip->mode == PlayMode::LOOP) {
      bool quick_exit = stop_requested && stop_style == StopStyle::QUICK;
      if (quick_exit) {
        phase = Phase::RETURN;
        return PlayerEvent::NONE;
      }
      if (frame == last && (stop_requested || time_up)) {
        phase = Phase::DONE; // il prossimo giro ripartirebbe dalla posa base
        return PlayerEvent::DONE;
      }
      frame = frame == last ? 0 : frame + 1;
      return PlayerEvent::NONE;
    }
    if (stop_requested) {
      phase = Phase::RETURN;
      return PlayerEvent::NONE;
    }
    if (frame < last) {
      frame++;
    }
    return frame == last ? reach_last_frame() : PlayerEvent::NONE;

  case Phase::HOLD:
    if (clip->mode == PlayMode::FORWARD) {
      phase = Phase::DONE;
      return PlayerEvent::DONE;
    }
    if (stop_requested || time_up) {
      phase = Phase::RETURN;
      return PlayerEvent::NONE;
    }
    if (clip->mode == PlayMode::HOLD && clip->hold_frames > 1) {
      int first = max(0, frame_count - clip->hold_frames);
      frame += hold_dir;
      if (frame <= first || frame >= last) {
        frame = constrain(frame, first, last);
        hold_dir = -hold_dir;
      }
    }
    return PlayerEvent::NONE;

  case Phase::RETURN:
    if (frame == 0) {
      phase = Phase::DONE;
      return PlayerEvent::DONE;
    }
    frame--;
    return PlayerEvent::NONE;

  case Phase::DONE:
    break;
  }
  return PlayerEvent::NONE;
}
} // namespace

void player_start(const Clip *next) {
  clip = next;
  frame_count = max(1, sprite_get_frame_count(clip->path));
  frame = clip->mode == PlayMode::BACKWARD ? frame_count - 1 : 0;
  phase = Phase::PLAY;
  stop_requested = false;
  started = last_step = millis();
  duration = clip->max_ms > 0 ? random(clip->min_ms, clip->max_ms) : 0;
  draw();
}

void player_stop(StopStyle style) {
  if (stop_requested && stop_style == StopStyle::QUICK) {
    return; // una volta veloce resta veloce
  }
  stop_requested = true;
  stop_style = style;
}

PlayerEvent player_update(bool background_moved) {
  if (clip == nullptr || phase == Phase::DONE) {
    return PlayerEvent::NONE;
  }
  PlayerEvent event = PlayerEvent::NONE;
  if (millis() - last_step >= step_interval()) {
    last_step = millis();
    event = step();
    if (event == PlayerEvent::DONE) {
      return event; // la prossima clip disegna da se'
    }
  }

  // il cartello fermo non si ridisegna, cancellerebbe la scritta
  bool sign_still = clip->mode == PlayMode::SIGN && phase == Phase::HOLD;
  bool changed = clip->path != drawn_path || frame != drawn_frame;
  if (event == PlayerEvent::SIGN_READY || (!sign_still && (changed || background_moved))) {
    draw();
  }
  return event;
}

const Clip *player_clip() { return clip; }
int player_frame() { return frame; }
bool player_holding() { return phase == Phase::HOLD; }
bool player_stopping() { return stop_requested; }

void player_area(int *x, int *y, int *w, int *h) {
  *x = area_x;
  *y = area_y;
  *w = area_w;
  *h = area_h;
}
