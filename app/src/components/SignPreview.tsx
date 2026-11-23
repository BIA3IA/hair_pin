import { useMemo } from 'react';
import { Image, PixelRatio, Text, View } from 'react-native';
import {
  animations,
  CROP_X,
  CROP_Y,
  SPRITE_SCALE,
  SPRITE_HEIGHT,
  SPRITE_WIDTH,
} from '@/bianca/frames';
import { layoutSign } from '@/bianca/sign-layout';

export function SignPreview({ text }: { text: string }) {
  const scale = SPRITE_SCALE / PixelRatio.get();
  const devicePixel = scale / 1.7;
  const layout = useMemo(() => layoutSign(text), [text]);
  const frames = animations.pull_card;
  return (
    <View className="items-center gap-3">
      <View
        accessibilityLabel={`Anteprima cartello: ${text}`}
        style={{ width: SPRITE_WIDTH / PixelRatio.get(), height: SPRITE_HEIGHT / PixelRatio.get() }}
      >
        <Image
          source={frames[frames.length - 1]}
          fadeDuration={0}
          style={{ width: '100%', height: '100%' }}
        />
        <View
          accessible={false}
          style={{
            position: 'absolute',
            left: (Math.trunc(58 * 1.7) / 1.7 - CROP_X) * scale,
            top: (Math.trunc(63 * 1.7) / 1.7 - CROP_Y) * scale,
          }}
        >
          {layout.runs.map((run, i) => (
            <View
              key={i}
              style={{
                position: 'absolute',
                left: run.x * devicePixel,
                top: run.y * devicePixel,
                width: run.width * devicePixel,
                height: devicePixel,
                backgroundColor: '#181818',
              }}
            />
          ))}
        </View>
      </View>
      {layout.unsupported.length > 0 && (
        <Text className="font-body text-heart text-base">
          Il font della molletta non contiene: {layout.unsupported.join(' ')}
        </Text>
      )}
      {!layout.fits && (
        <Text className="font-body text-heart text-base">
          La frase supera lo spazio del cartello: accorciala per evitare tagli sulla molletta.
        </Text>
      )}
    </View>
  );
}
