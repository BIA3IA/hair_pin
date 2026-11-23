#include "collegamento.h"
#include <BLEDevice.h>
#include <BLESecurity.h>
#include <BLEServer.h>

namespace {
const unsigned long PAIRING_WINDOW_MS = 120000; // nuovi abbinamenti solo appena accesa
const unsigned long STATUS_MIN_INTERVAL_MS = 10000;

portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
StaticQueue_t notify_queue_state;
uint8_t notify_queue_storage[12 * sizeof(NotifyPacket)];
QueueHandle_t notify_queue = nullptr;
TimePacket time_box;
uint32_t passkey_box = 0;
bool has_time = false, has_passkey = false;
bool paired = false;

BLEServer *server = nullptr;
BLECharacteristic *status_char = nullptr;
uint8_t last_status[STATUS_SIZE] = {};
unsigned long last_status_time = 0;
bool status_sent = false;

template <typename T> void put(T &box, bool &flag, const T &value) {
  portENTER_CRITICAL(&lock);
  box = value;
  flag = true;
  portEXIT_CRITICAL(&lock);
}

template <typename T> bool take(T &box, bool &flag, T *out) {
  portENTER_CRITICAL(&lock);
  bool had = flag;
  if (had) {
    *out = box;
    flag = false;
  }
  portEXIT_CRITICAL(&lock);
  return had;
}

class NotifyWrite : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c, ble_gap_conn_desc *) override {
    String value = c->getValue();
    NotifyPacket packet;
    if (decode_notify((const uint8_t *)value.c_str(), value.length(), &packet)) {
      xQueueSend(notify_queue, &packet, 0);
    }
  }
};

class TimeWrite : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c, ble_gap_conn_desc *) override {
    String value = c->getValue();
    TimePacket packet;
    if (decode_time((const uint8_t *)value.c_str(), value.length(), &packet)) {
      put(time_box, has_time, packet);
    }
  }
};

class Security : public BLESecurityCallbacks {
  void onPassKeyNotify(uint32_t pass_key) override { put(passkey_box, has_passkey, pass_key); }

  void onAuthenticationComplete(ble_gap_conn_desc *desc) override {
    if (desc->sec_state.encrypted && desc->sec_state.authenticated) {
      portENTER_CRITICAL(&lock);
      paired = true;
      portEXIT_CRITICAL(&lock);
    }
  }
};
} // namespace

void collegamento_init() {
  notify_queue = xQueueCreateStatic(12, sizeof(NotifyPacket), notify_queue_storage, &notify_queue_state);
  BLEDevice::init("Bianca");
  BLEDevice::setSecurityCallbacks(new Security());

  // codice di 6 cifre mostrato da Bianca, nuovo a ogni abbinamento
  BLESecurity::setCapability(ESP_IO_CAP_OUT);
  BLESecurity::setAuthenticationMode(true, true, true);
  BLESecurity::setPassKey(false);
  BLESecurity::regenPassKeyOnConnect(true);

  server = BLEDevice::createServer();
  server->advertiseOnDisconnect(true);
  BLEService *service = server->createService(BIANCA_SERVICE_UUID);

  const uint32_t secure_write =
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_AUTHEN;
  service->createCharacteristic(BIANCA_NOTIFY_UUID, secure_write)->setCallbacks(new NotifyWrite());
  service->createCharacteristic(BIANCA_TIME_UUID, secure_write)->setCallbacks(new TimeWrite());
  status_char = service->createCharacteristic(
      BIANCA_STATUS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_READ_AUTHEN |
                              BLECharacteristic::PROPERTY_NOTIFY);
  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BIANCA_SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.printf("Bluetooth pronto, heap libero %u\n", ESP.getFreeHeap());
}

void collegamento_update(const StatusPacket &status) {
  uint8_t encoded[STATUS_SIZE];
  encode_status(status, encoded);
  unsigned long now = millis();
  bool changed = !status_sent || memcmp(encoded, last_status, STATUS_SIZE) != 0;
  if (changed && now - last_status_time >= STATUS_MIN_INTERVAL_MS) {
    memcpy(last_status, encoded, STATUS_SIZE);
    status_char->setValue(encoded, STATUS_SIZE);
    if (server->getConnectedCount() > 0) {
      status_char->notify();
    }
    last_status_time = now;
    status_sent = true;
  }
}

bool collegamento_take_notify(NotifyPacket *out) { return xQueueReceive(notify_queue, out, 0) == pdTRUE; }

bool collegamento_take_time(TimePacket *out) { return take(time_box, has_time, out); }

bool collegamento_take_paired() {
  portENTER_CRITICAL(&lock);
  bool was_paired = paired;
  paired = false;
  portEXIT_CRITICAL(&lock);
  return was_paired;
}

bool collegamento_take_passkey(uint32_t *out) {
  if (!take(passkey_box, has_passkey, out)) {
    return false;
  }
  if (millis() > PAIRING_WINDOW_MS) {
    // fuori dalla finestra: niente codice, si chiude la connessione
    Serial.println("Abbinamento rifiutato: finestra chiusa");
    server->disconnect(server->getConnId());
    return false;
  }
  return true;
}

bool collegamento_connected() { return server && server->getConnectedCount() > 0; }
