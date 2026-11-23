import { useCallback, useEffect, useState } from 'react';
import { Pressable, Text, TextInput, View } from 'react-native';
import BiancaLink, { type BundlesState, type PhraseBundle } from '../../modules/bianca-link';
import { DEFAULT_PHRASE_COUNT } from '@/bianca/default-phrases';
import { phraseProblem } from '@/bianca/phrases';
import { BundleEditor } from './BundleEditor';
import { DefaultPhrases } from './DefaultPhrases';
import { PixelButton } from './PixelButton';
import { SignPreview } from './SignPreview';

type Props = {
  connected: boolean;
  sending: boolean;
  feedback: string;
  showText: (text: string) => void;
};

type Editing = { bundle: PhraseBundle | null } | null;

function Choice({
  label,
  selected,
  onPress,
}: {
  label: string;
  selected: boolean;
  onPress: () => void;
}) {
  return (
    <Pressable
      accessibilityRole="tab"
      accessibilityState={{ selected }}
      onPress={onPress}
      className={`min-h-11 flex-1 items-center justify-center rounded-full ${selected ? 'bg-violet' : ''}`}
    >
      <Text className={`font-body-bold text-base ${selected ? 'text-paper' : 'text-mist'}`}>
        {label}
      </Text>
    </Pressable>
  );
}

function Radio({ on }: { on: boolean }) {
  return (
    <View
      className={`size-6 items-center justify-center rounded-full border-2 ${on ? 'border-lilac' : 'border-plum-700'}`}
    >
      {on && <View className="bg-lilac size-3 rounded-full" />}
    </View>
  );
}

function BundleRow({
  name,
  detail,
  active,
  select,
  action,
  children,
}: {
  name: string;
  detail: string;
  active: boolean;
  select: () => void;
  action?: { label: string; onPress: () => void };
  children?: React.ReactNode;
}) {
  return (
    <View
      className={`rounded-3xl border-2 ${active ? 'border-lilac bg-plum-800' : 'border-plum-700 bg-plum-900'}`}
    >
      <View className="flex-row items-center gap-3 p-4">
        <Pressable
          accessibilityRole="radio"
          accessibilityState={{ checked: active }}
          accessibilityLabel={`Usa ${name}`}
          onPress={select}
          className="flex-1 flex-row items-center gap-3"
        >
          <Radio on={active} />
          <View className="flex-1">
            <Text className="font-body-bold text-paper text-base">{name}</Text>
            <Text className="font-body text-mist text-sm">{detail}</Text>
          </View>
        </Pressable>
        {action && (
          <Pressable
            accessibilityRole="button"
            onPress={action.onPress}
            className="bg-plum-950 min-h-10 justify-center rounded-full px-4"
          >
            <Text className="font-body-bold text-lilac text-sm">{action.label}</Text>
          </Pressable>
        )}
      </View>
      {children}
    </View>
  );
}

export function SignPanel({ connected, sending, feedback, showText }: Props) {
  const [tab, setTab] = useState<'text' | 'bundles'>('text');
  const [text, setText] = useState('Ti voglio bene');
  const [state, setState] = useState<BundlesState>({ active: null, bundles: [] });
  const [editing, setEditing] = useState<Editing>(null);
  const [showDefaults, setShowDefaults] = useState(false);
  const [error, setError] = useState('');
  const problem = phraseProblem(text);

  const refresh = useCallback(
    () =>
      BiancaLink.bundlesState()
        .then(setState)
        .catch((e: unknown) =>
          setError(e instanceof Error ? e.message : 'Impossibile leggere i bundle'),
        ),
    [],
  );
  useEffect(() => {
    let alive = true;
    BiancaLink.bundlesState()
      .then((next) => {
        if (alive) setState(next);
      })
      .catch(() => {
        if (alive) setError('Impossibile leggere i bundle');
      });
    return () => {
      alive = false;
    };
  }, []);

  async function choose(id: string | null) {
    setError('');
    try {
      await BiancaLink.setActiveBundle(id);
      await refresh();
    } catch (e) {
      setError(e instanceof Error ? e.message : 'Impossibile scegliere il bundle');
    }
  }

  if (editing) {
    const current = editing.bundle;
    return (
      <BundleEditor
        bundle={current}
        cancel={() => setEditing(null)}
        save={async (name, phrases) => {
          await BiancaLink.saveBundle(current?.id ?? null, name, phrases);
          await refresh();
          setEditing(null);
        }}
        remove={
          current
            ? async () => {
                await BiancaLink.deleteBundle(current.id);
                await refresh();
                setEditing(null);
              }
            : undefined
        }
      />
    );
  }

  return (
    <View className="gap-5">
      <View className="bg-plum-900 flex-row rounded-full p-1">
        <Choice label="Una frase" selected={tab === 'text'} onPress={() => setTab('text')} />
        <Choice label="Bundle" selected={tab === 'bundles'} onPress={() => setTab('bundles')} />
      </View>

      {tab === 'text' ? (
        <View className="gap-4">
          <SignPreview text={text} />
          <TextInput
            accessibilityLabel="Scritta del cartello"
            value={text}
            onChangeText={setText}
            maxLength={48}
            placeholder="Scrivi per Bianca"
            placeholderTextColor="#9C8FB0"
            className="font-body border-plum-700 bg-plum-900 text-paper rounded-2xl border-2 px-4 py-3 text-base"
          />
          {problem && text.trim() !== '' && (
            <Text className="font-body text-heart text-sm">{problem}</Text>
          )}
          <PixelButton
            label="Mostra su Bianca"
            disabled={!connected || problem !== null}
            busy={sending}
            onPress={() => showText(text.trim())}
          />
          <Text
            accessibilityLiveRegion="polite"
            className="font-body text-mist text-center text-sm"
          >
            {connected ? feedback : 'Collega Bianca dall’icona Bluetooth per inviarla.'}
          </Text>
        </View>
      ) : (
        <View className="gap-3">
          <Text className="font-body text-mist text-sm leading-5">
            Il bundle scelto va sulla molletta ogni volta che si collega. Quando è scollegata,
            Bianca usa le sue frasi.
          </Text>
          <BundleRow
            name="Frasi di Bianca"
            detail={`${DEFAULT_PHRASE_COUNT} frasi, già sulla molletta`}
            active={state.active === null}
            select={() => void choose(null)}
            action={{
              label: showDefaults ? 'Chiudi' : 'Vedi',
              onPress: () => setShowDefaults(!showDefaults),
            }}
          >
            {showDefaults && <DefaultPhrases />}
          </BundleRow>
          {state.bundles.map((bundle) => (
            <BundleRow
              key={bundle.id}
              name={bundle.name}
              detail={`${bundle.phrases.length} ${bundle.phrases.length === 1 ? 'frase' : 'frasi'}`}
              active={state.active === bundle.id}
              select={() => void choose(bundle.id)}
              action={{ label: 'Modifica', onPress: () => setEditing({ bundle }) }}
            />
          ))}
          {error !== '' && <Text className="font-body text-heart text-sm">{error}</Text>}
          <PixelButton
            label="Nuovo bundle"
            variant="secondary"
            onPress={() => setEditing({ bundle: null })}
          />
        </View>
      )}
    </View>
  );
}
