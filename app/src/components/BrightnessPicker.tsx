import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import BiancaLink from '../../modules/bianca-link';

const LEVELS = [30, 80, 140, 200, 255];

function nearest(value: number) {
  let best = 0;
  LEVELS.forEach((level, i) => {
    if (Math.abs(level - value) < Math.abs(LEVELS[best]! - value)) best = i;
  });
  return best;
}

export function BrightnessPicker({
  connected,
  brightness,
}: {
  connected: boolean;
  brightness?: number;
}) {
  const [chosen, setChosen] = useState<number | null>(null);
  const [error, setError] = useState('');

  // lo stato della molletta arriva in ritardo: si mostra subito la scelta
  const current = chosen ?? (brightness !== undefined ? nearest(brightness) : -1);

  async function choose(i: number) {
    const previous = chosen;
    setChosen(i);
    setError('');
    try {
      await BiancaLink.sendCommands([6], [String(LEVELS[i])]);
    } catch (e) {
      setChosen(previous);
      setError(e instanceof Error ? e.message : 'Invio non riuscito');
    }
  }

  return (
    <View className="gap-2">
      <Text className="font-body text-mist text-center text-sm">Luminosità</Text>
      <View className="flex-row items-end justify-center gap-3">
        {LEVELS.map((level, i) => (
          <Pressable
            key={level}
            accessibilityRole="button"
            accessibilityLabel={`Luminosità ${i + 1} di ${LEVELS.length}`}
            accessibilityState={{ disabled: !connected, selected: i === current }}
            disabled={!connected}
            onPress={() => void choose(i)}
            hitSlop={8}
            className="w-9 items-center justify-end"
            style={{ height: 44 }}
          >
            <View
              className={`w-full border-2 ${
                !connected
                  ? 'border-plum-700 bg-plum-800'
                  : i <= current
                    ? 'border-lilac bg-violet'
                    : 'border-plum-700 bg-plum-900'
              }`}
              style={{ height: 12 + i * 8 }}
            />
          </Pressable>
        ))}
      </View>
      {error !== '' && (
        <Text accessibilityLiveRegion="polite" className="font-body text-mist text-center text-sm">
          {error}
        </Text>
      )}
    </View>
  );
}
