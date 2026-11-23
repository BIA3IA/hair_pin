import { PNG } from "pngjs";
import fs from "node:fs";
import path from "node:path";

// Esporta le animazioni per l'app: ritaglio comune a tutte (cosi' Bianca non
// si sposta cambiando animazione) e ingrandimento intero a pixel nitidi.
const ANIMATIONS = fs.readdirSync(path.join(import.meta.dirname, "../data/Idle"))
  .filter((file) => file.endsWith(".bin"))
  .map((file) => file.slice(0, -4)).sort();
const SCALE = 6;
const FRAME_MS = 100;

const root = path.resolve(import.meta.dirname, "..");
const outDir = path.join(root, "app/assets/bianca");
const indexFile = path.join(root, "app/src/bianca/frames.ts");

const metadata = JSON.parse(fs.readFileSync(path.join(root, "sprites/metadata.json"), "utf8"));
const animations: Record<string, { frames: string[] }> = metadata.states[0].frames.animations;

const load = (file: string) => PNG.sync.read(fs.readFileSync(path.join(root, "sprites", file)));

const frames = Object.fromEntries(
  ANIMATIONS.map((name) => {
    const animation = animations[name];
    if (!animation) throw new Error(`Animazione sconosciuta: ${name}`);
    return [name, animation.frames.map(load)];
  }),
);

let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
for (const png of Object.values(frames).flat()) {
  for (let y = 0; y < png.height; y++) {
    for (let x = 0; x < png.width; x++) {
      if (png.data[(png.width * y + x) * 4 + 3] === 255) {
        minX = Math.min(minX, x);
        minY = Math.min(minY, y);
        maxX = Math.max(maxX, x);
        maxY = Math.max(maxY, y);
      }
    }
  }
}
const width = maxX - minX + 1;
const height = maxY - minY + 1;

function cropAndScale(png: PNG): PNG {
  const out = new PNG({ width: width * SCALE, height: height * SCALE });
  for (let y = 0; y < out.height; y++) {
    for (let x = 0; x < out.width; x++) {
      const src = ((minY + Math.floor(y / SCALE)) * png.width + minX + Math.floor(x / SCALE)) * 4;
      const dst = (y * out.width + x) * 4;
      png.data.copy(out.data, dst, src, src + 4);
    }
  }
  return out;
}

fs.mkdirSync(outDir, { recursive: true });
const lines: string[] = [];
for (const [name, pngs] of Object.entries(frames)) {
  fs.mkdirSync(path.join(outDir, name), { recursive: true });
  const requires = pngs.map((png, i) => {
    const file = `${String(i).padStart(2, "0")}.png`;
    const target = path.join(outDir, name, file);
    fs.writeFileSync(`${target}.tmp`, PNG.sync.write(cropAndScale(png)));
    fs.renameSync(`${target}.tmp`, target);
    return `require('../../assets/bianca/${name}/${file}')`;
  });
  lines.push(`  ${name}: [\n    ${requires.join(",\n    ")},\n  ],`);
}

fs.mkdirSync(path.dirname(indexFile), { recursive: true });
fs.writeFileSync(
  `${indexFile}.tmp`,
  `// Generato da tools/export_app_sprites.ts: non modificare a mano.
import type { ImageSourcePropType } from 'react-native';

export const CROP_X = ${minX};
export const CROP_Y = ${minY};
export const SPRITE_SCALE = ${SCALE};
export const SPRITE_WIDTH = ${width * SCALE};
export const SPRITE_HEIGHT = ${height * SCALE};
export const FRAME_MS = ${FRAME_MS};

export const animations = {
${lines.join("\n")}
} satisfies Record<string, ImageSourcePropType[]>;

export type AnimationName = keyof typeof animations;
`,
);
fs.renameSync(`${indexFile}.tmp`, indexFile);
// solo ora che l'indice non le cita piu' si possono togliere le animazioni rimosse
for (const entry of fs.readdirSync(outDir)) {
  if (!ANIMATIONS.includes(entry)) fs.rmSync(path.join(outDir, entry), { recursive: true });
}
console.log(`Esportate ${ANIMATIONS.join(", ")}: ${width * SCALE}x${height * SCALE} px`);
