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

    static bool tilted = false;
    static bool upside_down = false;
    static SensorShakeState shake_state = {false, false};

    tilted = is_tilted(angle, tilted);
    upside_down = is_upside_down(angle, upside_down);
    shake_state = is_shaking(magnitude, gyro_magnitude, shake_state.shaking);

    const bool step_detected = step_detection(vertical_accel);
    const bool walking = is_walking(step_detected, shake_state.rotating);

    SensorState state = {tilted, upside_down,
                         shake_state.shaking || shake_state.rotating, walking};

    return state;
  }

  return {false, false, false, false};
}