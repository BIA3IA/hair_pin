import { Pressable, Text, View } from 'react-native';

type Props = {
  label: string;
  onPress?: () => void;
  disabled?: boolean;
  busy?: boolean; // ignora i tocchi senza cambiare aspetto (invii brevi)
  variant?: 'primary' | 'secondary';
};

const FACE = {
  primary: 'border-lilac bg-violet group-active:bg-violet-bright',
  secondary: 'border-plum-700 bg-plum-900 group-active:bg-plum-800',
} as const;

export function PixelButton({
  label,
  onPress,
  disabled = false,
  busy = false,
  variant = 'primary',
}: Props) {
  return (
    <Pressable
      accessibilityRole="button"
      accessibilityState={{ disabled, busy }}
      disabled={disabled || busy}
      onPress={onPress}
      className="group relative"
    >
      <View className="bg-plum-950 absolute top-1 left-1 h-full w-full rounded-2xl" />
      <View
        className={`rounded-2xl border-2 px-5 py-3.5 ${
          disabled ? 'border-plum-700 bg-plum-800' : FACE[variant]
        }`}
      >
        <Text
          className={`font-pixel text-center text-lg tracking-wide ${
            disabled ? 'text-mist' : 'text-paper'
          }`}
        >
          {label}
        </Text>
      </View>
    </Pressable>
  );
}
