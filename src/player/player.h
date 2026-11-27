#pragma once
#include <Arduino.h>

// Le clip complete avanzano fino alla fine; solo le transizioni di posa
// che lo richiedono usano il ritorno inverso.
enum class PlayMode {
  FORWARD,  // avanti una volta, si ferma sull'ultimo frame
  ONCE,     // avanti e poi indietro fino alla posa base
  LOOP,     // ripete il giro; si ferma a fine giro
  HOLD,     // avanti, ondeggia sugli ultimi frame, poi torna indietro
  BACKWARD, // dall'ultimo frame al primo
  SIGN,     // tira fuori il cartello, lo tiene fermo, lo rimette via
};


struct Clip {
  const char *path;
  PlayMode mode;
  uint8_t hold_frames;
  uint32_t min_ms, max_ms; // quanto dura; 0 = finche' non la fermi
  uint16_t frame_ms;
};

enum class StopStyle {
  GENTLE, // il giro in corso finisce, il resto torna indietro con calma
  QUICK,  // torna indietro subito e veloce
};

enum class PlayerEvent { NONE, SIGN_READY, DONE };

void player_start(const Clip *clip);
void player_stop(StopStyle style);
PlayerEvent player_update(bool background_moved);
const Clip *player_clip();
int player_frame();
bool player_holding(); // ondeggia sul finale o tiene il cartello
bool player_stopping();
void player_area(int *x, int *y, int *w, int *h);
