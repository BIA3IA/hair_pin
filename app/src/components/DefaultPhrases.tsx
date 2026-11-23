import { useState } from 'react';
import { Pressable, Text, View } from 'react-native';
import { DEFAULT_PHRASE_GROUPS } from '@/bianca/default-phrases';

// le frasi della molletta, per categoria; si apre una categoria alla volta
export function DefaultPhrases() {
  const [open, setOpen] = useState<string | null>(DEFAULT_PHRASE_GROUPS[0]?.title ?? null);

  return (
    <View className="gap-2 px-3 pb-3">
      {DEFAULT_PHRASE_GROUPS.map(({ title, phrases }) => {
        const expanded = open === title;
        return (
          <View key={title} className="bg-plum-950 overflow-hidden rounded-2xl">
            <Pressable
              accessibilityRole="button"
              accessibilityState={{ expanded }}
              onPress={() => setOpen(expanded ? null : title)}
              className="min-h-12 flex-row items-center justify-between px-4"
            >
              <Text className="font-body-bold text-paper text-base">{title}</Text>
              <View className="flex-row items-center gap-3">
                <Text className="font-body text-mist text-sm">{phrases.length}</Text>
                <Text className="font-pixel text-lilac text-base">{expanded ? '−' : '+'}</Text>
              </View>
            </Pressable>
            {expanded &&
              phrases.map((phrase) => (
                <View key={phrase.text} className="border-plum-800 border-t px-4 py-3">
                  <Text className="font-body text-paper text-base">{phrase.text}</Text>
                  {phrase.when && (
                    <Text className="font-body text-mist text-xs">{phrase.when}</Text>
                  )}
                </View>
              ))}
          </View>
        );
      })}
    </View>
  );
}
