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

    qmi.getGyroscope(gyr.x, gyr.y, gyr.z);
    float gyro_magnitude = sqrtf(gyr.x * gyr.x + gyr.y * gyr.y + gyr.z * gyr.z);

    // mi ricordio il valore più alto visto negli ultimi ms, così un istante di
    // rotazione quasi nulla (l'inversione di un movimento) non fa credere che
    // la rotazione sia finita
    const unsigned long ROTATION_PEAK_HOLD_MS = 500;
    const float ROTATION_THRESHOLD = 90.0;
    unsigned long now = millis();

    static float gyro_peak = 0;
    static unsigned long gyro_peak_time = 0;

    if (gyro_magnitude > gyro_peak ||
        (now - gyro_peak_time) > ROTATION_PEAK_HOLD_MS) {
      gyro_peak = gyro_magnitude;
      gyro_peak_time = now;
    }

    const bool is_rotating = gyro_peak > ROTATION_THRESHOLD;
    const bool walking = is_walking(step_detected, is_rotating);

    if (step_detected) {
      Serial.printf("Step detected! vertical=%f | Walking: %d | GyroMag: "
                    "%.1f | Rotating: %d\n",
                    vertical_accel, walking, gyro_magnitude, is_rotating);
    }
    // Serial.printf("ACC x:%.2f y:%.2f z:%.2f | GYR x:%.2f y:%.2f z:%.2f | "
    //               "Tilted: %d | UpsideDown: %d | Shaking: %d | Step: %d\n",
    //               acc.x, acc.y, acc.z, gyr.x, gyr.y, gyr.z, tilted,
    //               upside_down, shaking, step_detected);
  }
}
