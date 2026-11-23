#include "shake.h"

// media "che dimentica": tau = dopo quanto pesa poco un valore vecchio
static float smooth(float current, float value, float dt, float tau) {
  return current + (value - current) * (dt / (tau + dt));
}

SensorShakeState is_shaking(float linear_accel, float gyro_magnitude,
                            float yaw_rate, float dt, bool currently_shaking) {
  // energia dell'accelerazione: un picco singolo non basta, deve durare
  const float SHAKE_ENTRY_G = 0.8;
  const float SHAKE_EXIT_G = 0.4;
  // camminando la testa ruota poco (< ~50 dps)
  const float ROTATING_DPS = 70.0;
  // "no" con la testa: almeno 3 inversioni sopra 100 dps in 1.2 s
  const float HEAD_SHAKE_DPS = 100.0;
  const unsigned long HEAD_SHAKE_WINDOW_MS = 1200;
  const unsigned long HEAD_SHAKE_HOLD_MS = 400;

  static float accel_energy = 0;
  static float gyro_energy = 0;
  static int last_sign = 0;
  static unsigned long reversals[3] = {};
  static unsigned long head_shake_until = 0;

  unsigned long now = millis();

  accel_energy = smooth(accel_energy, linear_accel * linear_accel, dt, 0.15);
  gyro_energy = smooth(gyro_energy, gyro_magnitude * gyro_magnitude, dt, 0.3);
  float accel_rms = sqrtf(accel_energy);
  float gyro_rms = sqrtf(gyro_energy);

  int sign = yaw_rate > HEAD_SHAKE_DPS ? 1 : yaw_rate < -HEAD_SHAKE_DPS ? -1 : 0;
  if (sign != 0 && sign != last_sign) {
    reversals[0] = reversals[1];
    reversals[1] = reversals[2];
    reversals[2] = now;
    last_sign = sign;
    if (reversals[0] != 0 && now - reversals[0] <= HEAD_SHAKE_WINDOW_MS) {
      head_shake_until = now + HEAD_SHAKE_HOLD_MS;
    }
  }

  SensorShakeState state;
  state.shaking = accel_rms > (currently_shaking ? SHAKE_EXIT_G : SHAKE_ENTRY_G);
  state.rotating = gyro_rms > ROTATING_DPS;
  state.head_shaking = now < head_shake_until;
  return state;
}
