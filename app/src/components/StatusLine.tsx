import { Text, View } from 'react-native';

type Props = {
  label: string;
  tone: 'off' | 'on' | 'waiting';
};

const DOT = {
  off: 'bg-mist',
  on: 'bg-lilac',
  waiting: 'bg-heart',
} as const;

export function StatusLine({ label, tone }: Props) {
  return (
    <View className="flex-row items-center gap-3">
      <View className={`size-2.5 ${DOT[tone]}`} />
      <Text className="font-body-bold text-paper text-base">{label}</Text>
    </View>
  );
}
