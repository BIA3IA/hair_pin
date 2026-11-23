import type { ReactNode } from 'react';
import { ScrollView } from 'react-native';
import { useSafeAreaInsets } from 'react-native-safe-area-context';

// pagina scorrevole che rispetta notch e barra di navigazione
export function Screen({ children }: { children: ReactNode }) {
  const insets = useSafeAreaInsets();
  return (
    <ScrollView
      className="bg-night flex-1"
      keyboardShouldPersistTaps="handled"
      contentContainerClassName="gap-6 px-6"
      contentContainerStyle={{ paddingTop: insets.top + 16, paddingBottom: insets.bottom + 32 }}
    >
      {children}
    </ScrollView>
  );
}
