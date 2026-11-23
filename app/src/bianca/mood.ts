// Soglie condivise con umore_level() nel firmware; STATUS contiene il punteggio 0–100.
export function moodLabel(value: number): string {
  if (value < 25) return 'Triste';
  if (value < 45) return 'Giù';
  if (value < 75) return 'Normale';
  return 'Felice';
}
