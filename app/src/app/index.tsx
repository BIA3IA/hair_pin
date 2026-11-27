import { router } from 'expo-router';
import { useState } from 'react';
import { Text, View } from 'react-native';
import type { AnimationName } from '@/bianca/frames';
import { ANIMATION_LABELS, ANIMATION_NAMES } from '@/bianca/labels';
import { BellIcon } from '@/components/BellIcon';
import { BatteryStatus } from '@/components/BatteryStatus';
import { BiancaSprite } from '@/components/BiancaSprite';
import { BluetoothIcon } from '@/components/BluetoothIcon';
import { BrightnessPicker } from '@/components/BrightnessPicker';
import { IconButton } from '@/components/IconButton';
import { NotificationPreferences } from '@/components/NotificationPreferences';
import { PixelButton } from '@/components/PixelButton';
import { Popup } from '@/components/Popup';
import { Screen } from '@/components/Screen';
import { Sparkles } from '@/components/Sparkles';
import { StatusLine } from '@/components/StatusLine';
import { useBianca } from '@/hooks/useBianca';
import { useSend } from '@/hooks/useSend';
import BiancaLink from '../../modules/bianca-link';

type Panel = 'link' | 'notifications';

const SURPRISES = ANIMATION_NAMES.filter((name) => name !== 'idle_1' && name !== 'pull_card');

export default function Home() {
  const link = useBianca();
  const [panel, setPanel] = useState<Panel | null>(null);
  const { sending, feedback, send } = useSend();
  const connected = link.phase === 'connected';
  const sleeping = link.phase === 'sleeping';
  const busy = link.phase === 'pairing' || link.phase === 'connecting';
  const play = (name: AnimationName) =>
    send([1], [name], `${ANIMATION_LABELS[name]}: inviata a Bianca`);
  const surprise = () => play(SURPRISES[Math.floor(Math.random() * SURPRISES.length)]!);

  return (
    <>
      <Screen>
        <View className="flex-row items-center justify-between">
          <Text accessibilityRole="header" className="font-pixel text-lilac text-4xl">
            Bianca
          </Text>
          <View className="flex-row gap-3">
            <IconButton label="Notifiche" onPress={() => setPanel('notifications')}>
              <BellIcon />
            </IconButton>
            <IconButton
              label={`Bluetooth: ${link.message}`}
              onPress={() => setPanel('link')}
              active={connected}
            >
              <BluetoothIcon />
            </IconButton>
          </View>
        </View>

        <View className="border-plum-700 bg-plum-950 items-center overflow-hidden rounded-[32px] border-2 pt-8 pb-6">
          <Sparkles />
          <BiancaSprite />
          <View className="bg-plum-700 h-1 w-36 rounded-full" />
        </View>
        <BatteryStatus connected={connected} percentage={link.status?.battery} />
        <BrightnessPicker connected={connected} brightness={link.status?.brightness} />
        {sleeping && (
          <Text
            accessibilityLiveRegion="polite"
            className="font-body text-mist text-center text-sm"
          >
            {link.message}
          </Text>
        )}

        <View className="gap-4">
          <PixelButton
            label="Sorpresa!"
            disabled={!connected}
            busy={sending}
            onPress={() => void surprise()}
          />
          <PixelButton
            label="Animazioni"
            variant="secondary"
            onPress={() => router.push('/animazioni')}
          />
          <PixelButton
            label="Il cartello"
            variant="secondary"
            onPress={() => router.push('/cartello')}
          />
          <PixelButton
            label="Fai dormire Bianca"
            variant="secondary"
            disabled={!connected}
            busy={sending}
            onPress={() => void BiancaLink.sleep().catch(() => undefined)}
          />
          {feedback !== '' && (
            <Text
              accessibilityLiveRegion="polite"
              className="font-body text-mist text-center text-sm"
            >
              {feedback}
            </Text>
          )}
        </View>
      </Screen>

      <Popup title="Collegamento" visible={panel === 'link'} close={() => setPanel(null)}>
        <StatusLine
          label={link.message}
          tone={connected ? 'on' : busy || sleeping ? 'waiting' : 'off'}
        />
        {!connected && !sleeping && (
          <Text className="font-body text-mist text-base leading-6">
            Per il primo abbinamento riaccendi Bianca e collegala entro due minuti: il codice
            compare sul suo cartello.
          </Text>
        )}
        <PixelButton
          label={connected ? 'Scollega' : busy ? 'Collegamento…' : 'Collega la molletta'}
          disabled={busy}
          onPress={() => void (connected ? link.disconnect() : link.connect())}
        />
        {busy && (
          <PixelButton label="Annulla" variant="secondary" onPress={() => void link.disconnect()} />
        )}
        {link.phase === 'error' && (
          <PixelButton
            label="Rimuovi associazione"
            variant="secondary"
            onPress={() => void link.forget()}
          />
        )}
      </Popup>
      <Popup title="Notifiche" visible={panel === 'notifications'} close={() => setPanel(null)}>
        <NotificationPreferences visible={panel === 'notifications'} />
      </Popup>
    </>
  );
}
