import { PageHeader } from '@/components/PageHeader';
import { Screen } from '@/components/Screen';
import { SignPanel } from '@/components/SignPanel';
import { useBianca } from '@/hooks/useBianca';
import { useSend } from '@/hooks/useSend';

export default function Sign() {
  const link = useBianca();
  const { sending, feedback, send } = useSend();
  return (
    <Screen>
      <PageHeader title="Il cartello" />
      <SignPanel
        connected={link.phase === 'connected'}
        sending={sending}
        feedback={feedback}
        showText={(text) => void send([2], [text], 'Scritta inviata a Bianca')}
      />
    </Screen>
  );
}
