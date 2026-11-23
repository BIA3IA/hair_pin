#include "imu.h"
#include "cadenza.h"
#include "inclinazione.h"
#include "passi.h"
#include "registro.h"
#include "shake.h"
#include <SensorQMI8658.hpp>
#include <Wire.h>

SensorQMI8658 qmi;

void imu_init() {
  if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println("QMI8658 non trovato, controlla i collegamenti I2C!");
    return;
  }

  // filtro del chip a ~16 Hz: i movimenti di una persona stanno sotto i 10
  qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G,
                          SensorQMI8658::ACC_ODR_125Hz,
                          SensorQMI8658::LPF_MODE_3);
  qmi.configGyroscope(SensorQMI8658::GYR_RANGE_512DPS,
                      SensorQMI8658::GYR_ODR_112_1Hz,
                      SensorQMI8658::LPF_MODE_3);
  qmi.enableAccelerometer();
  qmi.enableGyroscope();
}

static bool jolt = false;

struct Vec {
  float x, y, z;
};

static float dot(const Vec &a, const Vec &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static void follow(Vec &v, const Vec &target, float alpha) {
  v.x += (target.x - v.x) * alpha;
  v.y += (target.y - v.y) * alpha;
  v.z += (target.z - v.z) * alpha;
}

SensorState imu_update() {
  static SensorState last_state = {false, false, false, false, false};
  static bool started = false;
  static Vec gravity, tilt_gravity;
  static Vec gyro_bias = {0, 0, 0};
  static unsigned long last_time = 0;
  static bool tilted = false;
  static bool upside_down = false;
  static SensorShakeState shake_state = {false, false, false};

  if (!qmi.getDataReady()) {
    return last_state;
  }

  IMUdata acc, gyr;
  qmi.getAccelerometer(acc.x, acc.y, acc.z);
  qmi.getGyroscope(gyr.x, gyr.y, gyr.z);
  registro_sample(acc.x, acc.y, acc.z, gyr.x, gyr.y, gyr.z);

  Vec a = {acc.x, acc.y, acc.z};
  Vec raw_g = {gyr.x, gyr.y, gyr.z};
  unsigned long now = millis();
  if (!started) {
    gravity = tilt_gravity = a;
    last_time = now;
    started = true;
  }
  float dt = min((now - last_time) / 1000.0f, 0.2f);
  last_time = now;

  // gravita' stimata: il verticale vero, comunque sia messa la molletta
  follow(gravity, a, dt / (1.0f + dt));
  follow(tilt_gravity, a, dt / (0.15f + dt));

  // il giroscopio da fermo non legge mai 0: impara lo scarto quando e' immobile
  Vec drift = {raw_g.x - gyro_bias.x, raw_g.y - gyro_bias.y, raw_g.z - gyro_bias.z};
  Vec still = {a.x - gravity.x, a.y - gravity.y, a.z - gravity.z};
  if (dot(still, still) < 0.03f * 0.03f && dot(drift, drift) < 20.0f * 20.0f) {
    follow(gyro_bias, raw_g, dt / (2.0f + dt));
  }
  Vec g = {raw_g.x - gyro_bias.x, raw_g.y - gyro_bias.y, raw_g.z - gyro_bias.z};

  float g_norm = sqrtf(dot(gravity, gravity));
  Vec up = {gravity.x / g_norm, gravity.y / g_norm, gravity.z / g_norm};
  Vec linear = {a.x - gravity.x, a.y - gravity.y, a.z - gravity.z};
  float vertical = dot(linear, up);
  float linear_magnitude = sqrtf(dot(linear, linear));
  float yaw_rate = dot(g, up); // rotazione attorno alla verticale: il "no"
  float gyro_magnitude = sqrtf(dot(g, g));

  // botta secca o caduta libera (in caduta l'accelerometro legge ~0 g)
  static int falling_samples = 0;
  falling_samples = sqrtf(dot(a, a)) < 0.35f ? falling_samples + 1 : 0;
  if (linear_magnitude > 1.8f || falling_samples >= 3) {
    jolt = true;
  }

  float angle = tilt_angle(tilt_gravity.x, tilt_gravity.y, tilt_gravity.z);
  tilted = is_tilted(angle, tilted);
  upside_down = is_upside_down(angle, upside_down);
  shake_state = is_shaking(linear_magnitude, gyro_magnitude, yaw_rate, dt,
                           shake_state.shaking);

  const bool step_detected = step_detection(vertical);
  const bool walking = is_walking(step_detected, shake_state.rotating);
  const bool running = is_running();

  last_state = {tilted, upside_down,
                shake_state.shaking || shake_state.head_shaking, walking,
                running};
  return last_state;
}

int imu_take_steps() { return walking_take_steps(); }

bool imu_take_jolt() {
  bool j = jolt;
  jolt = false;
  return j;
}
