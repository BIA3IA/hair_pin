#pragma once
#include "protocollo.h"

// Servizio Bluetooth "Bianca". I callback del Bluetooth girano in un altro task:
// qui si mettono da parte gli eventi, il loop li ritira con le funzioni take.
void collegamento_init();
void collegamento_update(const StatusPacket &status);
bool collegamento_take_notify(NotifyPacket *out);
bool collegamento_take_time(TimePacket *out);
bool collegamento_take_passkey(uint32_t *out); // codice da mostrare sul cartello
bool collegamento_take_paired();               // abbinamento appena riuscito

bool collegamento_connected();
