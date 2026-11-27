#include <Arduino.h>
#pragma once

#define I2C_SDA 8
#define I2C_SCL 7

struct SensorState {
  bool tilted;
  bool upside_down;
  bool shaking;
  bool walking;
  bool running;
};

void imu_init();
SensorState imu_update(); // da chiamare a ritmo fisso (50 Hz)
bool imu_take_jolt();     // c'e' stata una botta o una caduta?
void imu_sleep();         // spegne i sensori fino al riavvio
