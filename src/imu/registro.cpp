#include "registro.h"
#include "../display/display.h"
#include <LittleFS.h>

static const char *LOG_PATH = "/imu_log.csv";
static const char *ARM_PATH = "/rec_arm";

static File log_file;
static unsigned long rec_start = 0;
static unsigned long rec_duration = 0;
static int unflushed = 0;

// pallino rosso in alto a sinistra mentre registra
static void show_dot(bool on) {
  gfx->fillRect(2, 2, 6, 6, on ? RGB565_RED : RGB565_BLACK);
}

static void start(unsigned long seconds) {
  if (log_file)
    log_file.close();
  log_file = LittleFS.open(LOG_PATH, "w");
  if (!log_file) {
    Serial.println("Impossibile creare il log!");
    return;
  }
  log_file.println("t,ax,ay,az,gx,gy,gz");
  rec_start = millis();
  rec_duration = seconds * 1000UL;
  unflushed = 0;
  show_dot(true);
  Serial.printf("Registro per %lu s\n", seconds);
}

static void stop() {
  if (!log_file)
    return;
  log_file.close();
  LittleFS.remove(ARM_PATH);
  show_dot(false);
  Serial.println("Registrazione finita");
}

void registro_init() {
  File arm = LittleFS.open(ARM_PATH, "r");
  if (!arm)
    return;
  unsigned long seconds = arm.readString().toInt();
  arm.close();
  if (seconds > 0)
    start(seconds);
}

static void dump() {
  File f = LittleFS.open(LOG_PATH, "r");
  if (!f) {
    Serial.println("Nessun log");
    return;
  }
  Serial.setTxTimeoutMs(200); // qui non si deve perdere nulla
  Serial.println("BEGIN_LOG");
  uint8_t buf[256];
  while (f.available()) {
    int n = f.read(buf, sizeof(buf));
    Serial.write(buf, n);
    Serial.flush();
  }
  f.close();
  Serial.println("END_LOG");
  Serial.flush();
  Serial.setTxTimeoutMs(0);
}

void registro_poll_serial() {
  static String line;
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') {
      if (line.length() < 32)
        line += c;
      continue;
    }
    line.trim();
    if (line.startsWith("rec")) {
      unsigned long seconds = line.length() > 3 ? line.substring(3).toInt() : 120;
      File arm = LittleFS.open(ARM_PATH, "w");
      arm.print(seconds);
      arm.close();
      start(seconds);
    } else if (line == "stop") {
      stop();
    } else if (line == "dump") {
      dump();
    }
    line = "";
  }

  if (log_file && millis() - rec_start >= rec_duration)
    stop();
}

void registro_sample(float ax, float ay, float az, float gx, float gy, float gz) {
  if (!log_file)
    return;
  log_file.printf("%lu,%.3f,%.3f,%.3f,%.1f,%.1f,%.1f\n", millis() - rec_start,
                  ax, ay, az, gx, gy, gz);
  if (++unflushed >= 50) { // circa una volta al secondo
    log_file.flush();
    unflushed = 0;
  }
}
