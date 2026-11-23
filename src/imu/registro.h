#include <Arduino.h>
#pragma once

// Registra i dati grezzi dell'IMU in /imu_log.csv, senza PC collegato.
// Dal monitor seriale:
//   rec 180  -> registra 180 s (riparte anche se si riaccende col powerbank)
//   stop     -> ferma
//   dump     -> stampa il file
void registro_init();
void registro_poll_serial();
void registro_sample(float ax, float ay, float az, float gx, float gy, float gz);
