import type { ReactNode } from 'react';
import { Modal, Pressable, ScrollView, Text, View } from 'react-native';
import { useSafeAreaInsets } from 'react-native-safe-area-context';

type Props = {
  title: string;
  visible: boolean;
  close: () => void;
  children: ReactNode;
};

// finestra centrata sopra uno sfondo scuro; toccare fuori la chiude
export function Popup({ title, visible, close, children }: Props) {
  const insets = useSafeAreaInsets();
  return (
    <Modal
      visible={visible}
      transparent
      animationType="fade"
      onRequestClose={close}
      statusBarTranslucent
    >
      <View
        className="flex-1 justify-center bg-black/70 px-5"
        style={{ paddingTop: insets.top + 20, paddingBottom: insets.bottom + 20 }}
      >
        <Pressable accessibilityLabel="Chiudi" onPress={close} className="absolute inset-0" />
        <View
          accessibilityViewIsModal
          className="border-plum-700 bg-night max-h-full rounded-3xl border-2 p-5"
        >
          <View className="mb-4 flex-row items-center justify-between gap-4">
            <Text accessibilityRole="header" className="font-pixel text-paper flex-1 text-2xl">
              {title}
            </Text>
            <Pressable
              accessibilityRole="button"
              accessibilityLabel="Chiudi"
              onPress={close}
              className="bg-plum-900 size-11 items-center justify-center rounded-full"
            >
              <Text className="text-lilac text-2xl leading-7">×</Text>
            </Pressable>
          </View>
          <ScrollView keyboardShouldPersistTaps="handled" contentContainerClassName="gap-4 pb-1">
            {children}
          </ScrollView>
        </View>
      </View>
    </Modal>
  );
}
