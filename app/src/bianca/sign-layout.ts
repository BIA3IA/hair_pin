import fontData from './sign-fonts.json' with { type: 'json' };

type Glyph = { w: number; h: number; x: number; y: number; advance: number; bits: string };
type Font = Record<string, Glyph>;
type Style = { font: Font; scale: number; lineH: number; boldX: number; boldY: number };
const styles: Style[] = [
  { font: fontData['5x8'], scale: 3, lineH: 24, boldX: 1, boldY: 1 },
  { font: fontData['5x8'], scale: 2, lineH: 16, boldX: 1, boldY: 1 },
  { font: fontData.helvB10, scale: 1, lineH: 16, boldX: 1, boldY: 0 },
  { font: fontData.helvB08, scale: 1, lineH: 11, boldX: 0, boldY: 0 },
];
// Stesse dimensioni intere di SIGN_W/H * SPRITE_SCALE sul display.
export const SIGN_WIDTH = 90;
export const SIGN_HEIGHT = 56;

function bounds(text: string, style: Style) {
  let cursor = 0,
    minX = 239,
    minY = 279,
    maxX = 0,
    maxY = 0;
  for (const char of text) {
    const glyph = style.font[String(char.codePointAt(0))];
    if (!glyph) continue;
    const x = cursor + glyph.x * style.scale;
    const y = -(glyph.h + glyph.y) * style.scale;
    minX = Math.min(minX, x);
    minY = Math.min(minY, y);
    maxX = Math.max(maxX, x + glyph.w * style.scale - 1);
    maxY = Math.max(maxY, y + glyph.h * style.scale - 1);
    cursor += glyph.advance * style.scale;
  }
  return {
    x: maxX >= minX ? minX : 0,
    y: maxY >= minY ? minY : 0,
    w: maxX >= minX ? maxX - minX + 1 : 0,
    h: maxY >= minY ? maxY - minY + 1 : 0,
  };
}

function lineBuffer(value: string): string {
  let size = 0,
    result = '';
  for (const char of value) {
    const point = char.codePointAt(0)!;
    const bytes = point <= 127 ? 1 : point <= 2047 ? 2 : point <= 65535 ? 3 : 4;
    if (size + bytes > 47) break;
    size += bytes;
    result += char;
  }
  return result;
}

function wrap(text: string, style: Style) {
  const lines = [''];
  const maxLines = Math.min(4, Math.trunc(SIGN_HEIGHT / style.lineH));
  let fits = true,
    forced = false,
    position = 0;
  while (position < text.length) {
    const match = /^[^ \n]*/.exec(text.slice(position))!;
    const originalWord = match[0];
    const word = lineBuffer(originalWord);
    const separator = text[position + originalWord.length];
    const current = lines[lines.length - 1]!;
    const candidate = current + (current ? ' ' : '') + word;
    if (current && (forced || bounds(candidate, style).w + style.boldX > SIGN_WIDTH - 8)) {
      if (lines.length === maxLines) {
        fits = false;
        lines[lines.length - 1] = lineBuffer(current + ' ' + word);
      } else lines.push(word);
    } else lines[lines.length - 1] = lineBuffer(candidate);
    forced = separator === '\n';
    position += originalWord.length + (separator ? 1 : 0);
  }
  for (const line of lines) if (bounds(line, style).w + style.boldX > SIGN_WIDTH - 8) fits = false;
  return { lines, fits };
}

export type PixelRun = { x: number; y: number; width: number };
export function layoutSign(text: string) {
  // NOTIFY sostituisce i controlli non stampabili con spazi.
  text = text.replace(/[\x00-\x09\x0b-\x1f]/g, ' ');
  let style = styles[3]!,
    result = wrap(text, style),
    styleIndex = 3;
  for (let i = 0; i < styles.length; i++) {
    const candidate = wrap(text, styles[i]!);
    if (candidate.fits) {
      style = styles[i]!;
      result = candidate;
      styleIndex = i;
      break;
    }
  }
  const boxes = result.lines.map((line) => bounds(line, style));
  const top = Math.min(...boxes.map((box, i) => i * style.lineH + box.y));
  const bottom = Math.max(...boxes.map((box, i) => i * style.lineH + box.y + box.h + style.boldY));
  const baseline = Math.trunc((SIGN_HEIGHT - (bottom - top)) / 2) - top;
  const pixels = new Uint8Array(SIGN_WIDTH * SIGN_HEIGHT);
  result.lines.forEach((line, index) => {
    const box = boxes[index]!;
    const startX = Math.trunc((SIGN_WIDTH - (box.w + style.boldX)) / 2) - box.x;
    for (let dx = 0; dx <= style.boldX; dx++)
      for (let dy = 0; dy <= style.boldY; dy++) {
        let cursor = startX + dx;
        for (const char of line) {
          const glyph = style.font[String(char.codePointAt(0))];
          if (!glyph) continue;
          for (let y = 0; y < glyph.h; y++)
            for (let x = 0; x < glyph.w; x++) {
              if (glyph.bits[y * glyph.w + x] !== '1') continue;
              for (let sy = 0; sy < style.scale; sy++)
                for (let sx = 0; sx < style.scale; sx++) {
                  const px = cursor + (glyph.x + x) * style.scale + sx;
                  const py =
                    baseline +
                    index * style.lineH +
                    dy -
                    (glyph.h + glyph.y - y) * style.scale +
                    sy;
                  if (px >= 0 && px < SIGN_WIDTH && py >= 0 && py < SIGN_HEIGHT)
                    pixels[py * SIGN_WIDTH + px] = 1;
                }
            }
          cursor += glyph.advance * style.scale;
        }
      }
  });
  const runs: PixelRun[] = [];
  for (let y = 0; y < SIGN_HEIGHT; y++)
    for (let x = 0; x < SIGN_WIDTH;) {
      if (!pixels[y * SIGN_WIDTH + x]) {
        x++;
        continue;
      }
      const start = x;
      while (x < SIGN_WIDTH && pixels[y * SIGN_WIDTH + x]) x++;
      runs.push({ x: start, y, width: x - start });
    }
  return {
    runs,
    lines: result.lines,
    fits: result.fits,
    styleIndex,
    unsupported: [
      ...new Set(
        Array.from(text).filter(
          (char) => char !== '\n' && !style.font[String(char.codePointAt(0))],
        ),
      ),
    ],
  };
}
