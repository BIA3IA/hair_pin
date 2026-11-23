import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import type { AnimationName } from '@/bianca/frames';
import { ANIMATION_GROUPS, ANIMATION_LABELS } from '@/bianca/labels';
import { BiancaSprite } from './BiancaSprite';
import { PixelButton } from './PixelButton';

type Props = {
  connected: boolean;
  sending: boolean;
  feedback: string;
  play: (name: AnimationName) => void;
};

export function AnimationPicker({ connected, sending, feedback, play }: Props) {
  const [selected, setSelected] = useState<AnimationName>('idle_1');

  return (
    <View className="gap-5">
      <View className="border-plum-700 bg-plum-950 items-center gap-2 rounded-3xl border-2 py-4">
        <BiancaSprite key={selected} animation={selected} />
        <Text className="font-pixel text-lilac text-xl">{ANIMATION_LABELS[selected]}</Text>
      </View>

      <PixelButton
        label="Fallo fare a Bianca"
        disabled={!connected}
        busy={sending}
        onPress={() => play(selected)}
      />
      <Text accessibilityLiveRegion="polite" className="font-body text-mist text-center text-sm">
        {connected
          ? feedback || 'Parte una volta, poi Bianca torna a fare di testa sua.'
          : 'Collega Bianca dall’icona Bluetooth per inviarla.'}
      </Text>

      {ANIMATION_GROUPS.map(({ group, title, names }) => (
        <View key={group} className="gap-3">
          <Text accessibilityRole="header" className="font-pixel text-lilac text-lg">
            {title}
          </Text>
          <View className="flex-row flex-wrap justify-between gap-y-2.5">
            {names.map((name) => {
              const active = name === selected;
              return (
                <Pressable
                  key={name}
                  accessibilityRole="button"
                  accessibilityState={{ selected: active }}
                  onPress={() => setSelected(name)}
                  className={`min-h-12 w-[48.5%] items-center justify-center rounded-2xl border-2 px-3 py-2.5 ${
                    active ? 'border-lilac bg-plum-800' : 'border-plum-700 bg-plum-900'
                  }`}
                >
                  <Text
                    numberOfLines={1}
                    className={`font-body text-center text-base ${active ? 'text-paper' : 'text-mist'}`}
                  >
                    {ANIMATION_LABELS[name]}
                  </Text>
                </Pressable>
              );
            })}
          </View>
        </View>
      ))}
    </View>
  );
}
