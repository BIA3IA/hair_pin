#include <LittleFS.h>

void sprite_init() {
  if (!LittleFS.begin()) {
    Serial.println("Errore montaggio LittleFS!");
    return;
  }

  File f = LittleFS.open("/Idle/idle_1.bin", "r");
  if (!f) {
    Serial.println("File non trovato!");
    return;
  }

  uint8_t header[6];
  f.read(header, 6);
  f.close();

  uint16_t width =
      header[0] | (header[1] << 8); // little-endian: byte basso prima
  uint16_t height = header[2] | (header[3] << 8);
  uint16_t frameCount = header[4] | (header[5] << 8);
  Serial.printf("width=%d height=%d frames=%d\n", width, height, frameCount);
}