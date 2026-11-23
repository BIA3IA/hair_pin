import { useState } from 'react';
import BiancaLink from '../../modules/bianca-link';

// invio di comandi alla molletta con stato e messaggio per l'interfaccia
export function useSend() {
  const [sending, setSending] = useState(false);
  const [feedback, setFeedback] = useState('');

  async function send(actions: number[], texts: string[], done: string) {
    setSending(true);
    setFeedback('');
    try {
      await BiancaLink.sendCommands(actions, texts);
      setFeedback(done);
    } catch (error) {
      setFeedback(error instanceof Error ? error.message : 'Invio non riuscito');
    } finally {
      setSending(false);
    }
  }

  return { sending, feedback, send };
}
