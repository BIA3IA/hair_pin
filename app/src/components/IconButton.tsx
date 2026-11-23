import type { ReactNode } from 'react';
import { Pressable, View } from 'react-native';

type Props = {
  label: string;
  onPress: () => void;
  children: ReactNode;
  active?: boolean; // pallino acceso
};

export function IconButton({ label, onPress, children, active }: Props) {
  return (
    <Pressable
      accessibilityRole="button"
      accessibilityLabel={label}
      onPress={onPress}
      className="border-plum-700 bg-plum-900 active:bg-plum-800 size-14 items-center justify-center rounded-full border-2"
    >
      {children}
      {active !== undefined && (
        <View
          className={`border-night absolute top-0.5 right-0.5 size-3.5 rounded-full border-2 ${
            active ? 'bg-lilac' : 'bg-mist'
          }`}
        />
      )}
    </Pressable>
  );
}
