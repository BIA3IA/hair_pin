import { bitmapPixels, PixelGlyph } from './PixelGlyph';

// la campanella del cartello della molletta
const BELL = bitmapPixels([0x18, 0x3c, 0x7e, 0x7e, 0x7e, 0xff, 0x18, 0x00]);

export function BellIcon() {
  return <PixelGlyph pixels={BELL} size={8} scale={3} color="#b79ced" />;
}
