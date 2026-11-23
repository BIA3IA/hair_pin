import { useState } from 'react';
import { Pressable, Text, TextInput, View } from 'react-native';
import type { PhraseBundle } from '../../modules/bianca-link';
import { BUNDLE_MAX_PHRASES, phraseProblem } from '@/bianca/phrases';
import { PixelButton } from './PixelButton';
import { SignPreview } from './SignPreview';

type Props = {
  bundle: PhraseBundle | null; // null = nuovo
  save: (name: string, phrases: string[]) => Promise<void>;
  remove?: () => Promise<void>;
  cancel: () => void;
};

const INPUT =
  'font-body border-plum-700 bg-plum-900 text-paper rounded-2xl border-2 px-4 py-3 text-base';

export function BundleEditor({ bundle, save, remove, cancel }: Props) {
  const [name, setName] = useState(bundle?.name ?? '');
  const [phrases, setPhrases] = useState<string[]>(bundle?.phrases ?? []);
  const [draft, setDraft] = useState('');
  const [preview, setPreview] = useState<number | null>(null);
  const [error, setError] = useState('');
  const [saving, setSaving] = useState(false);

  const draftProblem = draft.trim() ? phraseProblem(draft) : null;
  const full = phrases.length >= BUNDLE_MAX_PHRASES;
  const previewText = draft.trim() || (preview !== null ? phrases[preview] : phrases.at(-1)) || '';

  function add() {
    if (draftProblem || !draft.trim() || full) return;
    setPhrases([...phrases, draft.trim()]);
    setDraft('');
    setPreview(null);
  }

  async function run(action: () => Promise<void>) {
    setSaving(true);
    setError('');
    try {
      await action();
    } catch (e) {
      setError(e instanceof Error ? e.message : 'Operazione non riuscita');
      setSaving(false);
    }
  }

  return (
    <View className="gap-4">
      <TextInput
        accessibilityLabel="Nome del bundle"
        value={name}
        onChangeText={setName}
        maxLength={30}
        placeholder="Nome, per esempio Giornate no"
        placeholderTextColor="#9C8FB0"
        className={INPUT}
      />

      <SignPreview text={previewText} />

      <View className="flex-row gap-2">
        <TextInput
          accessibilityLabel="Nuova frase"
          value={draft}
          onChangeText={setDraft}
          onSubmitEditing={add}
          returnKeyType="done"
          maxLength={48}
          placeholder={full ? 'Bundle pieno' : 'Nuova frase'}
          placeholderTextColor="#9C8FB0"
          editable={!full}
          className={`${INPUT} flex-1`}
        />
        <Pressable
          accessibilityRole="button"
          accessibilityLabel="Aggiungi frase"
          disabled={!draft.trim() || draftProblem !== null || full}
          onPress={add}
          className={`bg-violet border-lilac size-14 items-center justify-center rounded-2xl border-2 ${
            !draft.trim() || draftProblem !== null || full ? 'opacity-40' : ''
          }`}
        >
          <Text className="font-pixel text-paper text-2xl">+</Text>
        </Pressable>
      </View>
      {draftProblem && <Text className="font-body text-heart text-sm">{draftProblem}</Text>}

      <Text className="font-body text-mist text-sm">
        {phrases.length} {phrases.length === 1 ? 'frase' : 'frasi'} · tocca una frase per vederla
        sul cartello
      </Text>
      <View className="gap-2">
        {phrases.map((phrase, index) => (
          <View
            key={`${index}-${phrase}`}
            className={`flex-row items-center rounded-2xl border-2 ${
              index === preview ? 'border-lilac bg-plum-800' : 'border-plum-700 bg-plum-900'
            }`}
          >
            <Pressable
              accessibilityRole="button"
              accessibilityLabel={`Anteprima: ${phrase}`}
              onPress={() => setPreview(index)}
              className="flex-1 px-4 py-3"
            >
              <Text className="font-body text-paper text-base">{phrase}</Text>
            </Pressable>
            <Pressable
              accessibilityRole="button"
              accessibilityLabel={`Togli: ${phrase}`}
              onPress={() => {
                setPhrases(phrases.filter((_, i) => i !== index));
                setPreview(null);
              }}
              className="size-12 items-center justify-center"
            >
              <Text className="text-mist text-xl">×</Text>
            </Pressable>
          </View>
        ))}
      </View>

      {error !== '' && <Text className="font-body text-heart text-sm">{error}</Text>}
      <PixelButton
        label={saving ? 'Salvataggio…' : 'Salva bundle'}
        disabled={saving || !name.trim() || phrases.length === 0}
        onPress={() => void run(() => save(name.trim(), phrases))}
      />
      <View className="flex-row gap-3">
        <View className="flex-1">
          <PixelButton label="Annulla" variant="secondary" onPress={cancel} disabled={saving} />
        </View>
        {remove && (
          <View className="flex-1">
            <PixelButton
              label="Elimina"
              variant="secondary"
              onPress={() => void run(remove)}
              disabled={saving}
            />
          </View>
        )}
      </View>
    </View>
  );
}
