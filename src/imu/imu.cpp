#include "imu.h"
#include "cadenza.h"
#include "inclinazione.h"
#include "passi.h"
#include "shake.h"
#include <SensorQMI8658.hpp>
#include <Wire.h>

SensorQMI8658 qmi;

// Sensore
void imu_init() {
  if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("QMI8658 non trovato, controlla i collegamenti I2C!");
  }

  qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
  qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G,
                          SensorQMI8658::ACC_ODR_1000Hz,
                          SensorQMI8658::LPF_MODE_0);
  qmi.configGyroscope(SensorQMI8658::GYR_RANGE_512DPS,
                      SensorQMI8658::GYR_ODR_896_8Hz,
                      SensorQMI8658::LPF_MODE_3);
  qmi.enableAccelerometer();
  qmi.enableGyroscope();
}

SensorState imu_update() {
  IMUdata acc, gyr;

  if (qmi.getDataReady()) {
    qmi.getAccelerometer(acc.x, acc.y, acc.z);
    qmi.getGyroscope(gyr.x, gyr.y, gyr.z);

    float angle = tilt_angle(acc.x, acc.y, acc.z);
    float magnitude = accel_magnitude(acc.x, acc.y, acc.z);
    float vertical_accel = fabsf(acc.y - 1.0f);
    float gyro_magnitude = sqrtf(gyr.x * gyr.x + gyr.y * gyr.y + gyr.z * gyr.z);

    unsigned long now = millis();

    static float gyro_peak = 0;
    static unsigned long gyro_peak_time = 0;

    static bool tilted = false;
    static bool upside_down = false;
    static bool shaking = false;

    tilted = is_tilted(angle, tilted);
    upside_down = is_upside_down(angle, upside_down);
    shaking = is_shaking(magnitude, shaking);

    const bool step_detected = step_detection(vertical_accel);

    if (gyro_magnitude > gyro_peak ||
        (now - gyro_peak_time) > ROTATION_PEAK_HOLD_MS) {
      gyro_peak = gyro_magnitude;
      gyro_peak_time = now;
    }

    const bool is_rotating = gyro_peak > ROTATION_THRESHOLD;
    bool is_shaken_signal = shaking || is_rotating;
    const bool walking = is_walking(step_detected, is_rotating);

    SensorState state = {tilted, upside_down, is_shaken_signal, walking};

    return state;
  }

  return {false, false, false, false}; // Default state if no data is ready
}
