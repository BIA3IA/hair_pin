import { useEffect, useState } from 'react';
import { PermissionsAndroid } from 'react-native';
import BiancaLink from '../../modules/bianca-link';

export function useBianca() {
  const [state, setState] = useState(() => BiancaLink.getState());
  useEffect(() => {
    const subscription = BiancaLink.addListener('onState', setState);
    return () => subscription.remove();
  }, []);

  async function connect() {
    try {
      const permission = PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT;
      if ((await PermissionsAndroid.request(permission)) !== PermissionsAndroid.RESULTS.GRANTED) {
        setState({
          phase: 'error',
          message: 'Consenti i dispositivi vicini nelle impostazioni dell’app.',
          status: null,
        });
        return;
      }
      await PermissionsAndroid.request(PermissionsAndroid.PERMISSIONS.POST_NOTIFICATIONS);
      await BiancaLink.connect();
    } catch (error) {
      setState({
        phase: 'error',
        message: error instanceof Error ? error.message : 'Collegamento non riuscito',
        status: null,
      });
    }
  }
  return {
    ...state,
    connect,
    disconnect: () => BiancaLink.disconnect(),
    forget: () => BiancaLink.forget(),
  };
}
