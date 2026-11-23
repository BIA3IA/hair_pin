import type { AnimationName } from '@/bianca/frames';
import { ANIMATION_LABELS } from '@/bianca/labels';
import { AnimationPicker } from '@/components/AnimationPicker';
import { PageHeader } from '@/components/PageHeader';
import { Screen } from '@/components/Screen';
import { useBianca } from '@/hooks/useBianca';
import { useSend } from '@/hooks/useSend';

export default function Animations() {
  const link = useBianca();
  const { sending, feedback, send } = useSend();
  const play = (name: AnimationName) =>
    void send([1], [name], `${ANIMATION_LABELS[name]}: inviata a Bianca`);
  return (
    <Screen>
      <PageHeader title="Animazioni" />
      <AnimationPicker
        connected={link.phase === 'connected'}
        sending={sending}
        feedback={feedback}
        play={play}
      />
    </Screen>
  );
}
