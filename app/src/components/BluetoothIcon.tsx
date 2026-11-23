import { View } from 'react-native';

const strokes = [
  [10, 0, 10, 28],
  [10, 0, 19, 7],
  [19, 7, 1, 21],
  [10, 28, 19, 21],
  [19, 21, 1, 7],
] as const;
export function BluetoothIcon() {
  return (
    <View accessible={false} style={{ width: 20, height: 28 }}>
      {strokes.map(([x1, y1, x2, y2], i) => (
        <View
          key={i}
          className="bg-lilac"
          style={{
            position: 'absolute',
            left: (x1 + x2) / 2 - Math.hypot(x2 - x1, y2 - y1) / 2,
            top: (y1 + y2) / 2 - 1,
            width: Math.hypot(x2 - x1, y2 - y1),
            height: 2,
            borderRadius: 1,
            transform: [{ rotate: `${(Math.atan2(y2 - y1, x2 - x1) * 180) / Math.PI}deg` }],
          }}
        />
      ))}
    </View>
  );
}
