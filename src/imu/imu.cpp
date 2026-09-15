#include "cadenza.h"
#include "imu.h"
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
  qmi.configGyroscope(SensorQMI8658::GYR_RANGE_64DPS,
                      SensorQMI8658::GYR_ODR_896_8Hz,
                      SensorQMI8658::LPF_MODE_3);
  qmi.enableAccelerometer();
  qmi.enableGyroscope();
}

void imu_read_print() {
  IMUdata acc, gyr;
  if (qmi.getDataReady()) {
    qmi.getAccelerometer(acc.x, acc.y, acc.z);

    float angle = tilt_angle(acc.x, acc.y, acc.z);

    static bool tilted = false;
    static bool upside_down = false;

    tilted = is_tilted(angle, tilted);
    upside_down = is_upside_down(angle, upside_down);

    float magnitude = accel_magnitude(acc.x, acc.y, acc.z);
    static bool shaking = false;
    shaking = is_shaking(magnitude, shaking);

    float vertical_accel = fabsf(acc.y - 1.0f);
    const bool step_detected = step_detection(vertical_accel);
    const bool walking = is_walking(step_detected);

    qmi.getGyroscope(gyr.x, gyr.y, gyr.z);

    if (step_detected) {
      Serial.printf("Step detected! vertical=%f | Walking: %d\n",
                    vertical_accel, walking);
    }
    // Serial.printf("ACC x:%.2f y:%.2f z:%.2f | GYR x:%.2f y:%.2f z:%.2f | "
    //               "Tilted: %d | UpsideDown: %d | Shaking: %d | Step: %d\n",
    //               acc.x, acc.y, acc.z, gyr.x, gyr.y, gyr.z, tilted,
    //               upside_down, shaking, step_detected);
  }
}
