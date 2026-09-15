#include "imu.h"
#include "inclinazione.h"
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

    qmi.getGyroscope(gyr.x, gyr.y, gyr.z);
    Serial.printf("ACC x:%.2f y:%.2f z:%.2f | GYR x:%.2f y:%.2f z:%.2f | "
                  "Tilted: %d | UpsideDown: %d",
                  acc.x, acc.y, acc.z, gyr.x, gyr.y, gyr.z, tilted,
                  upside_down);
  }
}
