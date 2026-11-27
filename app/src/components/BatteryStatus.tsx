import { Text, View } from 'react-native';
import { bitmapPixels, PixelGlyph } from './PixelGlyph';

const BATTERY = bitmapPixels([0x18, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x00]);

export function BatteryStatus({
  connected,
  percentage,
}: {
  connected: boolean;
  percentage?: number;
}) {
  const known = connected && percentage !== undefined && percentage >= 0 && percentage <= 100;
  const label = !connected
    ? 'Batteria · collega Bianca'
    : known
      ? `Batteria stimata · ${percentage}%`
      : 'Batteria non disponibile';

  return (
    <View
      accessible
      accessibilityLabel={label}
      className="flex-row items-center justify-center gap-2"
    >
      <PixelGlyph pixels={BATTERY} size={8} scale={2} color="#b79ced" />
      <Text className="font-body text-mist text-sm">{label}</Text>
    </View>
  );
}
