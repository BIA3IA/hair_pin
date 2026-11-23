import { useEffect } from 'react';
import { View, type DimensionValue } from 'react-native';
import Animated, {
  Easing,
  cancelAnimation,
  useAnimatedStyle,
  useSharedValue,
  withDelay,
  withRepeat,
  withTiming,
} from 'react-native-reanimated';
import { SHAPE_COLORS, shapePixels, type Shape } from '@/bianca/shapes';
import { PixelGlyph } from './PixelGlyph';

type Spark = { shape: Shape; size: number; left: DimensionValue; top: DimensionValue };

// ai lati e sotto, per non coprire Bianca
const SPARKS: readonly Spark[] = [
  { shape: 'heart', size: 16, left: '6%', top: '12%' },
  { shape: 'star', size: 14, left: '12%', top: '60%' },
  { shape: 'star', size: 16, left: '80%', top: '8%' },
  { shape: 'heart', size: 14, left: '84%', top: '56%' },
  { shape: 'heart', size: 14, left: '26%', top: '84%' },
  { shape: 'star', size: 14, left: '66%', top: '82%' },
];
const PIXEL = 2;

function Drifting({ spark, index }: { spark: Spark; index: number }) {
  const x = useSharedValue(-6);
  const y = useSharedValue(-9);

  useEffect(() => {
    const ease = Easing.inOut(Easing.sin);
    x.value = withDelay(
      index * 350,
      withRepeat(withTiming(6, { duration: 3200 + index * 260, easing: ease }), -1, true),
    );
    y.value = withDelay(
      index * 200,
      withRepeat(withTiming(9, { duration: 2700 + index * 310, easing: ease }), -1, true),
    );
    return () => {
      cancelAnimation(x);
      cancelAnimation(y);
    };
  }, [index, x, y]);

  const style = useAnimatedStyle(() => ({
    transform: [{ translateX: x.value }, { translateY: y.value }],
  }));

  return (
    <Animated.View style={[{ position: 'absolute', left: spark.left, top: spark.top }, style]}>
      <PixelGlyph
        pixels={shapePixels(spark.shape, spark.size)}
        size={spark.size}
        scale={PIXEL}
        color={SHAPE_COLORS[spark.shape]}
      />
    </Animated.View>
  );
}

export function Sparkles() {
  return (
    <View pointerEvents="none" className="absolute inset-0">
      {SPARKS.map((spark, index) => (
        <Drifting key={index} spark={spark} index={index} />
      ))}
    </View>
  );
}
