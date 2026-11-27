#pragma once
#include <stddef.h>
#include <stdint.h>

// Protocollo Bluetooth v1 fra la molletta e l'app. Interi little-endian.
// Le stesse costanti stanno nell'app (modules/bianca-link/.../Protocol.kt).

#define BIANCA_SERVICE_UUID "1c0017b5-a1e0-46a8-b545-40745eeb96d1"
#define BIANCA_NOTIFY_UUID "cdcb32b1-2a88-4051-b7d3-adbc1f51e60e"
#define BIANCA_TIME_UUID "f4b0c127-e20d-433d-886e-d2494f3a8535"
#define BIANCA_STATUS_UUID "6b4747a6-1e7f-48c9-aa59-ddd21c5a844d"

constexpr uint8_t PROTOCOL_VERSION = 1;
constexpr size_t NOTIFY_TEXT_MAX = 48;
constexpr uint8_t ICON_COUNT = 12;
constexpr uint8_t STATUS_VERSION = 3;
constexpr size_t STATUS_SIZE = 4;
constexpr uint8_t BATTERY_UNKNOWN = 0xFF;
constexpr uint8_t MIN_BRIGHTNESS = 10;

// azioni dei comandi: 1 animazione, 2 cartello, 3-4-5 frasi, 6 luminosita', 7 dormi
constexpr uint8_t ACTION_BRIGHTNESS = 6;
constexpr uint8_t ACTION_SLEEP = 7;

// NOTIFY: versione, icona, flag, lunghezza, testo UTF-8
struct NotifyPacket {
  uint8_t icon;
  uint8_t flags;
  char text[NOTIFY_TEXT_MAX + 1];
};

// TIME: anno, mese, giorno, ora, minuti, secondi (ora locale del telefono)
struct TimePacket {
  uint16_t year;
  uint8_t month, day, hour, minute, second;
};

// STATUS: versione, flag (bit 0 = dorme), batteria, luminosita'
struct StatusPacket {
  bool sleeping;
  uint8_t battery;
  uint8_t brightness;
};

bool valid_animation(const char *name);

bool decode_notify(const uint8_t *data, size_t len, NotifyPacket *out);
// luminosita' scritta in cifre, MIN_BRIGHTNESS..255
bool parse_brightness(const char *text, uint8_t *out);

bool decode_time(const uint8_t *data, size_t len, TimePacket *out);
// scrive STATUS_SIZE byte in out
void encode_status(const StatusPacket &status, uint8_t *out);
