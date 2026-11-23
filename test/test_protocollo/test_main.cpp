#include "ble/protocollo.h"
#include <string.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

static void notify_valido() {
  const uint8_t data[] = {1, 5, 0, 6, 'G', 'i', 'u', 'l', 'i', 'a'};
  NotifyPacket p;
  TEST_ASSERT_TRUE(decode_notify(data, sizeof(data), &p));
  TEST_ASSERT_EQUAL_UINT8(5, p.icon);
  TEST_ASSERT_EQUAL_STRING("Giulia", p.text);
}

static void notify_versione_sconosciuta() {
  const uint8_t data[] = {2, 0, 0, 1, 'x'};
  NotifyPacket p;
  TEST_ASSERT_FALSE(decode_notify(data, sizeof(data), &p));
}

static void notify_lunghezza_incoerente() {
  const uint8_t corto[] = {1, 0, 0, 5, 'a', 'b'};
  const uint8_t lungo[] = {1, 0, 0, 1, 'a', 'b'};
  NotifyPacket p;
  TEST_ASSERT_FALSE(decode_notify(corto, sizeof(corto), &p));
  TEST_ASSERT_FALSE(decode_notify(lungo, sizeof(lungo), &p));
}

static void notify_troppo_lungo() {
  uint8_t data[4 + NOTIFY_TEXT_MAX + 1] = {1, 0, 0, NOTIFY_TEXT_MAX + 1};
  memset(data + 4, 'a', NOTIFY_TEXT_MAX + 1);
  NotifyPacket p;
  TEST_ASSERT_FALSE(decode_notify(data, sizeof(data), &p));
}

static void notify_toglie_caratteri_di_controllo() {
  const uint8_t data[] = {1, 0, 0, 3, 'a', 0x07, 'b'};
  NotifyPacket p;
  TEST_ASSERT_TRUE(decode_notify(data, sizeof(data), &p));
  TEST_ASSERT_EQUAL_STRING("a b", p.text);
}

static void notify_icona_sconosciuta_diventa_nessuna() {
  const uint8_t data[] = {1, 200, 0, 1, 'a'};
  NotifyPacket p;
  TEST_ASSERT_TRUE(decode_notify(data, sizeof(data), &p));
  TEST_ASSERT_EQUAL_UINT8(0, p.icon);
}

static void time_valido() {
  const uint8_t data[] = {0xEA, 0x07, 11, 23, 14, 5, 9}; // 2026-11-23 14:05:09
  TimePacket t;
  TEST_ASSERT_TRUE(decode_time(data, sizeof(data), &t));
  TEST_ASSERT_EQUAL_UINT16(2026, t.year);
  TEST_ASSERT_EQUAL_UINT8(23, t.day);
  TEST_ASSERT_EQUAL_UINT8(9, t.second);
}

static void time_fuori_intervallo() {
  const uint8_t mese13[] = {0xEA, 0x07, 13, 1, 0, 0, 0};
  const uint8_t ora24[] = {0xEA, 0x07, 1, 1, 24, 0, 0};
  const uint8_t corto[] = {0xEA, 0x07, 1, 1, 0, 0};
  TimePacket t;
  TEST_ASSERT_FALSE(decode_time(mese13, sizeof(mese13), &t));
  TEST_ASSERT_FALSE(decode_time(ora24, sizeof(ora24), &t));
  TEST_ASSERT_FALSE(decode_time(corto, sizeof(corto), &t));
}

static void status_codificato() {
  uint8_t out[STATUS_SIZE];
  encode_status({72, 4200, true, BATTERY_UNKNOWN}, out);
  const uint8_t expected[] = {1, 72, 0x68, 0x10, 0, 0, 1, 0xFF};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, STATUS_SIZE);
}

static void icone_chat() {
  NotifyPacket packet;
  const uint8_t wa[] = {1, 9, 0, 1, 'x'};
  const uint8_t tg[] = {1, 10, 0, 1, 'x'};
  const uint8_t ig[] = {1, 11, 0, 0};
  TEST_ASSERT_TRUE(decode_notify(wa, sizeof(wa), &packet));
  TEST_ASSERT_EQUAL_UINT8(9, packet.icon);
  TEST_ASSERT_TRUE(decode_notify(tg, sizeof(tg), &packet));
  TEST_ASSERT_EQUAL_UINT8(10, packet.icon);
  TEST_ASSERT_TRUE(decode_notify(ig, sizeof(ig), &packet));
  TEST_ASSERT_EQUAL_UINT8(11, packet.icon);
  TEST_ASSERT_EQUAL_STRING("", packet.text);
}

static void comandi_remoti() {
  NotifyPacket packet;
  const uint8_t animation[] = {1, 0, 1, 4, 'w','a','v','e'};
  const uint8_t path[] = {1, 0, 1, 3, '.','.','/'};
  const uint8_t unknown[] = {1, 0, 9, 0};
  const uint8_t empty_phrase[] = {1, 0, 4, 0};
  const uint8_t begin[] = {1, 0, 3, 0};
  const uint8_t commit[] = {1, 0, 5, 0};
  TEST_ASSERT_TRUE(decode_notify(animation, sizeof(animation), &packet));
  TEST_ASSERT_EQUAL_UINT8(1, packet.flags);
  TEST_ASSERT_FALSE(decode_notify(path, sizeof(path), &packet));
  TEST_ASSERT_FALSE(decode_notify(unknown, sizeof(unknown), &packet));
  TEST_ASSERT_FALSE(decode_notify(empty_phrase, sizeof(empty_phrase), &packet));
  TEST_ASSERT_TRUE(decode_notify(begin, sizeof(begin), &packet));
  TEST_ASSERT_TRUE(decode_notify(commit, sizeof(commit), &packet));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(icone_chat);
  RUN_TEST(comandi_remoti);
  RUN_TEST(notify_valido);
  RUN_TEST(notify_versione_sconosciuta);
  RUN_TEST(notify_lunghezza_incoerente);
  RUN_TEST(notify_troppo_lungo);
  RUN_TEST(notify_toglie_caratteri_di_controllo);
  RUN_TEST(notify_icona_sconosciuta_diventa_nessuna);
  RUN_TEST(time_valido);
  RUN_TEST(time_fuori_intervallo);
  RUN_TEST(status_codificato);
  return UNITY_END();
}
