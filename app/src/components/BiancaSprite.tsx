import { useEffect, useState } from 'react';
import { Image, PixelRatio, View } from 'react-native';
import {
  animations,
  FRAME_MS,
  SPRITE_HEIGHT,
  SPRITE_WIDTH,
  type AnimationName,
} from '@/bianca/frames';

type Props = {
  animation?: AnimationName;
};

// 1 pixel dell'immagine = 1 pixel fisico, cosi' la pixel art resta nitida
const width = SPRITE_WIDTH / PixelRatio.get();
const height = SPRITE_HEIGHT / PixelRatio.get();

export function BiancaSprite({ animation = 'idle_1' }: Props) {
  // un nome non piu' esistente (es. dopo un aggiornamento al volo) ripiega su idle_1
  const frames = animations[animation] ?? animations.idle_1;
  const [tick, setTick] = useState(0);
  const frame = tick % frames.length;

  useEffect(() => {
    const timer = setInterval(() => setTick((t) => t + 1), FRAME_MS);
    return () => clearInterval(timer);
  }, []);

  // tutti i frame restano montati: cambiare solo l'opacita' evita lampi mentre si decodificano
  return (
    <View style={{ width, height }} accessibilityLabel="Bianca">
      {frames.map((source, i) => (
        <Image
          key={i}
          source={source}
          fadeDuration={0}
          style={{ position: 'absolute', width, height, opacity: i === frame ? 1 : 0 }}
        />
      ))}
    </View>
  );
}
