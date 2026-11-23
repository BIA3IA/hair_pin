import { router } from 'expo-router';
import { Pressable, Text, View } from 'react-native';

export function PageHeader({ title }: { title: string }) {
  return (
    <View className="flex-row items-center gap-4">
      <Pressable
        accessibilityRole="button"
        accessibilityLabel="Indietro"
        onPress={() => router.back()}
        className="border-plum-700 bg-plum-900 active:bg-plum-800 size-12 items-center justify-center rounded-full border-2"
      >
        <Text className="font-pixel text-lilac text-xl">‹</Text>
      </Pressable>
      <Text accessibilityRole="header" className="font-pixel text-lilac text-3xl">
        {title}
      </Text>
    </View>
  );
}
