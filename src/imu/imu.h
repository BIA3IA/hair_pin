#include <Arduino.h>

#define I2C_SDA 8
#define I2C_SCL 7

#define ROTATION_PEAK_HOLD_MS 500
#define ROTATION_THRESHOLD 90.0

struct SensorState {
  bool tilted;
  bool upside_down;
  bool shaking;
  bool walking;
};

void imu_init();
SensorState imu_update();