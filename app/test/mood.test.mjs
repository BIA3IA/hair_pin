import { test } from 'node:test';
import assert from 'node:assert/strict';
import { moodLabel } from '../src/bianca/mood.ts';

test('STATUS contiene un punteggio, anche quando supera il vecchio indice 0–3', () => {
  assert.equal(moodLabel(72), 'Normale');
  assert.equal(moodLabel(0), 'Triste');
  assert.equal(moodLabel(100), 'Felice');
});

test('le soglie di umore coincidono con quelle del firmware', () => {
  for (const [value, label] of [
    [24, 'Triste'],
    [25, 'Giù'],
    [44, 'Giù'],
    [45, 'Normale'],
    [74, 'Normale'],
    [75, 'Felice'],
  ]) {
    assert.equal(moodLabel(value), label);
  }
});
