#include "protocollo.h"
#include <string.h>

bool valid_animation(const char *name) {
  static const char *names[] = {"beat_you","bouncing","coffe","cuddled","dance","equilibrio","falling_asleep","falling_tired","get_phone","grumpy","idle_1","idle_2","idle_3","little_jump_in_place","looking_right","lose_balance","no_head","pull_card","pull_frame_glow","pull_orb","push","running","sad","shake","slide","startled","steady","strange_pose","thoughtful","upsidedown","walking","wave","yanked"};
  for (const char *candidate : names) if (strcmp(name, candidate) == 0) return true;
  return false;
}

bool decode_notify(const uint8_t *data, size_t len, NotifyPacket *out) {
  if (len < 4 || data[0] != PROTOCOL_VERSION) {
    return false;
  }
  size_t text_len = data[3];
  if (text_len > NOTIFY_TEXT_MAX || len != 4 + text_len) {
    return false;
  }
  out->icon = data[1] < ICON_COUNT ? data[1] : 0;
  out->flags = data[2];
  for (size_t i = 0; i < text_len; i++) {
    uint8_t c = data[4 + i];
    out->text[i] = c < 0x20 && c != '\n' ? ' ' : (char)c; // niente caratteri di controllo
  }
  out->text[text_len] = '\0';
  if (out->flags > 5) return false;
  if (out->flags == 1 && !valid_animation(out->text)) return false;
  if ((out->flags == 2 || out->flags == 4) && text_len == 0) return false;
  if ((out->flags == 3 || out->flags == 5) && text_len != 0) return false;
  return true;
}

bool decode_time(const uint8_t *data, size_t len, TimePacket *out) {
  if (len != 7) {
    return false;
  }
  TimePacket t;
  t.year = data[0] | (data[1] << 8);
  t.month = data[2];
  t.day = data[3];
  t.hour = data[4];
  t.minute = data[5];
  t.second = data[6];
  bool valid = t.year >= 2024 && t.year <= 2099 && t.month >= 1 && t.month <= 12 &&
               t.day >= 1 && t.day <= 31 && t.hour < 24 && t.minute < 60 && t.second < 60;
  if (valid) {
    *out = t;
  }
  return valid;
}

void encode_status(const StatusPacket &status, uint8_t *out) {
  out[0] = PROTOCOL_VERSION;
  out[1] = status.mood;
  out[2] = status.steps & 0xFF;
  out[3] = (status.steps >> 8) & 0xFF;
  out[4] = (status.steps >> 16) & 0xFF;
  out[5] = (status.steps >> 24) & 0xFF;
  out[6] = status.sleeping ? 1 : 0;
  out[7] = status.battery;
}
