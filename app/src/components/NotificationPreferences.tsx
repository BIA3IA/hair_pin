import { useEffect, useState } from 'react';
import { AppState, Pressable, Text, View } from 'react-native';
import BiancaLink, {
  type NotificationApp,
  type NotificationState,
} from '../../modules/bianca-link';
import { PixelButton } from './PixelButton';

const MODES: { mode: NotificationApp['mode']; label: string }[] = [
  { mode: 'app', label: 'Nome app' },
  { mode: 'sender', label: 'Mittente' },
  { mode: 'preview', label: '+ messaggio' },
];

function Switch({
  on,
  label,
  onPress,
  disabled,
}: {
  on: boolean;
  label: string;
  onPress: () => void;
  disabled: boolean;
}) {
  return (
    <Pressable
      accessibilityRole="switch"
      accessibilityLabel={label}
      accessibilityState={{ checked: on, disabled }}
      disabled={disabled}
      onPress={onPress}
      className={`h-8 w-14 justify-center rounded-full px-1 ${on ? 'bg-violet items-end' : 'bg-plum-700 items-start'}`}
    >
      <View className={`size-6 rounded-full ${on ? 'bg-paper' : 'bg-mist'}`} />
    </Pressable>
  );
}

function AppCard({
  app,
  saving,
  update,
}: {
  app: NotificationApp;
  saving: boolean;
  update: (enabled: boolean, mode: NotificationApp['mode']) => void;
}) {
  const logoOnly = app.pkg === 'com.instagram.android';
  return (
    <View
      className={`gap-3 rounded-3xl border-2 p-4 ${app.enabled ? 'border-lilac bg-plum-800' : 'border-plum-700 bg-plum-900'}`}
    >
      <View className="flex-row items-center justify-between gap-3">
        <Text className="font-body-bold text-paper flex-1 text-base">{app.label}</Text>
        <Switch
          on={app.enabled}
          label={`Notifiche di ${app.label}`}
          disabled={saving}
          onPress={() => update(!app.enabled, app.mode)}
        />
      </View>
      {app.enabled && !logoOnly && (
        <View className="bg-plum-950 flex-row rounded-full p-1">
          {MODES.map(({ mode, label }) => {
            const selected = app.mode === mode;
            return (
              <Pressable
                key={mode}
                accessibilityRole="radio"
                accessibilityState={{ checked: selected, disabled: saving }}
                disabled={saving}
                onPress={() => update(app.enabled, mode)}
                className={`min-h-9 flex-1 items-center justify-center rounded-full ${selected ? 'bg-violet' : ''}`}
              >
                <Text className={`font-body text-sm ${selected ? 'text-paper' : 'text-mist'}`}>
                  {label}
                </Text>
              </Pressable>
            );
          })}
        </View>
      )}
      {app.enabled && (
        <Text className="font-body text-mist text-xs leading-4">
          {logoOnly
            ? 'Solo il logo, senza nomi né messaggi.'
            : app.mode === 'preview'
              ? 'Il messaggio sarà leggibile da chi ti sta intorno. Nei gruppi solo il nome del gruppo.'
              : 'Nei gruppi compare solo il nome del gruppo.'}
        </Text>
      )}
    </View>
  );
}

export function NotificationPreferences({ visible }: { visible: boolean }) {
  const [state, setState] = useState<NotificationState | null>(null);
  const [error, setError] = useState('');
  const [saving, setSaving] = useState(false);

  useEffect(() => {
    if (!visible) return;
    let alive = true;
    const load = () =>
      BiancaLink.notificationState()
        .then((next) => {
          if (alive) setState(next);
        })
        .catch((e: unknown) => {
          if (alive)
            setError(e instanceof Error ? e.message : 'Impossibile leggere le impostazioni');
        });
    void load();
    // tornando dalle impostazioni di Android il permesso puo' essere cambiato
    const subscription = AppState.addEventListener('change', (phase) => {
      if (phase === 'active') void load();
    });
    return () => {
      alive = false;
      subscription.remove();
    };
  }, [visible]);

  async function update(app: NotificationApp, enabled: boolean, mode: NotificationApp['mode']) {
    setSaving(true);
    setError('');
    try {
      await BiancaLink.setNotificationRule(app.pkg, enabled, mode);
      setState(await BiancaLink.notificationState());
    } catch (e) {
      setError(e instanceof Error ? e.message : 'Salvataggio non riuscito');
    } finally {
      setSaving(false);
    }
  }

  if (state && !state.access) {
    return (
      <View className="gap-4">
        <Text className="font-body text-paper text-base leading-6">
          Per mostrare le notifiche sul cartello, Bianca deve poterle leggere. Attiva “Bianca” nella
          schermata che si apre.
        </Text>
        <PixelButton
          label="Consenti l’accesso"
          onPress={() =>
            void BiancaLink.openNotificationSettings().catch((e) => setError(String(e)))
          }
        />
        {error !== '' && <Text className="font-body text-heart text-sm">{error}</Text>}
      </View>
    );
  }

  return (
    <View className="gap-3">
      <Text className="font-body text-mist text-sm leading-5">
        Scegli quali app possono comparire sul cartello di Bianca.
      </Text>
      {state?.apps.map((app) => (
        <AppCard
          key={app.pkg}
          app={app}
          saving={saving}
          update={(enabled, mode) => void update(app, enabled, mode)}
        />
      ))}
      {state && state.apps.length === 0 && (
        <Text className="font-body text-mist text-sm">
          Nessuna app supportata installata: funziona con WhatsApp, Telegram e Instagram.
        </Text>
      )}
      {state?.pending && (
        <Text className="font-body text-mist text-xs">
          Una notifica aspetta che la molletta si colleghi (al massimo 10 minuti).
        </Text>
      )}
      {error !== '' && (
        <Text accessibilityLiveRegion="polite" className="font-body text-heart text-sm">
          {error}
        </Text>
      )}
    </View>
  );
}
