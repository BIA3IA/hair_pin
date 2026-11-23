// Le stesse forme dello sfondo della molletta (src/display/background.cpp).
export type Shape = 'heart' | 'star';
export type Pixel = readonly [x: number, y: number];

const HEART_COLOR = '#b5698c';
const STAR_COLOR = '#94318c';
export const SHAPE_COLORS: Record<Shape, string> = { heart: HEART_COLOR, star: STAR_COLOR };

function line(pixels: Set<string>, x: number, y: number, endX: number, endY: number) {
  const dx = Math.abs(endX - x);
  const dy = -Math.abs(endY - y);
  const sx = x < endX ? 1 : -1;
  const sy = y < endY ? 1 : -1;
  let error = dx + dy;
  for (;;) {
    pixels.add(`${x},${y}`);
    if (x === endX && y === endY) return;
    const twice = 2 * error;
    if (twice >= dy) {
      error += dy;
      x += sx;
    }
    if (twice <= dx) {
      error += dx;
      y += sy;
    }
  }
}

function outline(shape: Shape, size: number): Pixel[] {
  const n = size - 1;
  const t = (value: number) => Math.trunc(value);
  const points =
    shape === 'star'
      ? [
          [t(n / 2), 0],
          [t((n * 4) / 5), n],
          [0, t((n * 2) / 5)],
          [n, t((n * 2) / 5)],
          [t(n / 5), n],
          [t(n / 2), 0],
        ]
      : [
          [t(n / 2), t(n / 5)],
          [t(n / 4), 0],
          [0, t(n / 5)],
          [0, t((n * 2) / 5)],
          [t(n / 2), n],
          [n, t((n * 2) / 5)],
          [n, t(n / 5)],
          [t((n * 3) / 4), 0],
          [t(n / 2), t(n / 5)],
        ];
  const pixels = new Set<string>();
  for (let i = 0; i + 1 < points.length; i++) {
    const [x, y] = points[i]!;
    const [endX, endY] = points[i + 1]!;
    line(pixels, x!, y!, endX!, endY!);
  }
  return [...pixels].map((key) => key.split(',').map(Number) as unknown as Pixel);
}

const cache = new Map<string, Pixel[]>();
export function shapePixels(shape: Shape, size: number): Pixel[] {
  const key = `${shape}-${size}`;
  let pixels = cache.get(key);
  if (!pixels) {
    pixels = outline(shape, size);
    cache.set(key, pixels);
  }
  return pixels;
}
