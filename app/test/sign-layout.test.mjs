import { test } from 'node:test';
import assert from 'node:assert/strict';
import fixtures from './sign-reference.json' with { type: 'json' };
import { layoutSign, SIGN_WIDTH, SIGN_HEIGHT } from '../src/bianca/sign-layout.ts';

for (const fixture of fixtures) {
  test(`il cartello coincide pixel per pixel con il layout C++: ${JSON.stringify(fixture.text)}`, () => {
    const pixels = Array(SIGN_WIDTH * SIGN_HEIGHT).fill('0');
    for (const run of layoutSign(fixture.text).runs) {
      for (let x = run.x; x < run.x + run.width; x++) pixels[run.y * SIGN_WIDTH + x] = '1';
    }
    assert.equal(pixels.join(''), fixture.pixels);
  });
}
test('avvisa se una parola non entra nel cartello', () =>
  assert.equal(layoutSign('W'.repeat(48)).fits, false));
test('segnala i caratteri assenti nei font del display', () => {
  assert.deepEqual(layoutSign('Caffè').unsupported, []);
  assert.deepEqual(layoutSign('Ciao 💜').unsupported, ['💜']);
});
