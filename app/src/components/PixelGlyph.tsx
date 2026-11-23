import { memo } from 'react';
import { View } from 'react-native';
import type { Pixel } from '@/bianca/shapes';

type Props = {
  pixels: readonly Pixel[];
  size: number; // lato della griglia in pixel
  scale: number; // dp per pixel
  color: string;
};

// disegna una griglia di pixel pieni, nitida a qualsiasi densita'
export const PixelGlyph = memo(function PixelGlyph({ pixels, size, scale, color }: Props) {
  return (
    <View accessible={false} style={{ width: size * scale, height: size * scale }}>
      {pixels.map(([x, y]) => (
        <View
          key={`${x},${y}`}
          style={{
            position: 'absolute',
            left: x * scale,
            top: y * scale,
            width: scale,
            height: scale,
            backgroundColor: color,
          }}
        />
      ))}
    </View>
  );
});

// icona 8x8 da righe di bit, come le icone del cartello
export function bitmapPixels(rows: readonly number[]): Pixel[] {
  return rows.flatMap((row, y) =>
    Array.from({ length: 8 }, (_, x) => x).flatMap((x) =>
      row & (0x80 >> x) ? [[x, y] as const] : [],
    ),
  );
}
