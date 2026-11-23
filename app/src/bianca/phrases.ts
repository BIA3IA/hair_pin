import { layoutSign } from './sign-layout';

export const PHRASE_MAX_BYTES = 48;
export const BUNDLE_MAX_PHRASES = 64; // quante ne tiene la RAM della molletta

export function utf8Length(value: string) {
  let size = 0;
  for (const char of value) {
    const point = char.codePointAt(0)!;
    size += point <= 0x7f ? 1 : point <= 0x7ff ? 2 : point <= 0xffff ? 3 : 4;
  }
  return size;
}

// null se la frase si puo' mostrare sul cartello cosi' com'e'
export function phraseProblem(text: string): string | null {
  const phrase = text.trim();
  if (!phrase) return 'Scrivi qualcosa';
  if (utf8Length(phrase) > PHRASE_MAX_BYTES) return 'Troppo lunga per il cartello';
  const layout = layoutSign(phrase);
  if (layout.unsupported.length > 0) {
    return `La molletta non ha questi caratteri: ${layout.unsupported.join(' ')}`;
  }
  if (!layout.fits) return 'Non ci sta sul cartello';
  return null;
}
