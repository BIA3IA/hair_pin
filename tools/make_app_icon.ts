import { PNG } from "pngjs";
import fs from "node:fs";
import path from "node:path";

// Icona dell'app in pixel art, nello stile delle icone del telefono:
// cornice viola ad angoli tagliati, fiocco, luna, cuori, nuvole e Bianca al centro.
const root = path.resolve(import.meta.dirname, "..");
const N = 128; // griglia di pixel "grossi"

type RGBA = [number, number, number, number];
const hex = (h: string, a = 255): RGBA => [parseInt(h.slice(1, 3), 16), parseInt(h.slice(3, 5), 16), parseInt(h.slice(5, 7), 16), a];
const C = {
  glow: hex("#e9d5ff"),
  bright: hex("#b57bff"),
  deep: hex("#5b21b6"),
  fill: hex("#120a1d"),
  cloudLight: hex("#a855f7"),
  cloud: hex("#7e22ce"),
  cloudDark: hex("#4c1d95"),
  pink: hex("#f0abfc"),
  star: hex("#d8b4fe"),
  moon: hex("#e9d5ff"),
  outline: hex("#c084fc"),
};

const grid: (RGBA | null)[] = new Array(N * N).fill(null);
const set = (x: number, y: number, c: RGBA) => {
  if (x >= 0 && y >= 0 && x < N && y < N) grid[y * N + x] = c;
};
const get = (x: number, y: number) => (x >= 0 && y >= 0 && x < N && y < N ? grid[y * N + x] : null);

// ottagono: quadrato [8, 120) con angoli tagliati, rientrato di inset
const LO = 8, HI = 120, CUT = 16;
function inside(x: number, y: number, inset: number) {
  const lo = LO + inset, hi = HI - inset, cut = CUT;
  if (x < lo || y < lo || x >= hi || y >= hi) return false;
  const dx = Math.min(x - lo, hi - 1 - x), dy = Math.min(y - lo, hi - 1 - y);
  return dx + dy >= cut - inset * 0.4;
}
for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
  if (inside(x, y, 7)) set(x, y, C.fill);
  else if (inside(x, y, 5)) set(x, y, C.deep);
  else if (inside(x, y, 2)) set(x, y, C.bright);
  else if (inside(x, y, 0)) set(x, y, C.glow);
}

// stelline sul fondo
for (const [x, y] of [[24, 30], [40, 18], [100, 52], [22, 64], [108, 76], [86, 24], [30, 84]]) {
  set(x, y, C.star);
  if ((x + y) % 3 === 0) { set(x - 1, y, C.star); set(x + 1, y, C.star); set(x, y - 1, C.star); set(x, y + 1, C.star); }
}

// Bianca dal primo frame di idle_1, con un bordo chiaro come le icone del telefono
const sprite = PNG.sync.read(fs.readFileSync(path.join(root, "sprites/Idle/animations/idle_1/frame_000.png")));
const SX = 50, SY = 22, SW = 68, SH = 100, OX = 30, OY = 26; // continua fin sotto le nuvole
const body: boolean[] = new Array(N * N).fill(false);
for (let y = 0; y < SH; y++) for (let x = 0; x < SW; x++) {
  const i = ((SY + y) * sprite.width + SX + x) * 4;
  if (sprite.data[i + 3] === 255 && inside(OX + x, OY + y, 7)) body[(OY + y) * N + OX + x] = true;
}
for (let y = 1; y < N - 1; y++) for (let x = 1; x < N - 1; x++) {
  if (body[y * N + x]) continue;
  const near = body[y * N + x - 1] || body[y * N + x + 1] || body[(y - 1) * N + x] || body[(y + 1) * N + x];
  if (near && inside(x, y, 7)) set(x, y, C.outline);
}
for (let y = 0; y < SH; y++) for (let x = 0; x < SW; x++) {
  const i = ((SY + y) * sprite.width + SX + x) * 4;
  if (body[(OY + y) * N + OX + x]) set(OX + x, OY + y, [sprite.data[i], sprite.data[i + 1], sprite.data[i + 2], 255]);
}

// nuvole in basso, sopra il busto di Bianca
// nuvole basse e larghe: ellissi schiacciate
const puff = (cx: number, cy: number, rx: number, ry: number, c: RGBA) => {
  for (let y = -ry; y <= ry; y++) for (let x = -rx; x <= rx; x++)
    if ((x * x) / (rx * rx) + (y * y) / (ry * ry) <= 1 && inside(cx + x, cy + y, 7)) set(cx + x, cy + y, c);
};
for (const [cx, cy, rx, ry] of [[20, 109, 11, 6], [40, 106, 13, 7], [62, 108, 12, 6], [84, 105, 13, 7], [106, 108, 12, 6]]) {
  puff(cx, cy + 2, rx, ry, C.cloudDark);
  puff(cx, cy, rx - 1, ry - 1, C.cloud);
  puff(cx - 3, cy - 2, Math.max(3, rx - 6), Math.max(1, ry - 4), C.cloudLight);
}

// luna crescente in alto a destra
for (let y = -8; y <= 8; y++) for (let x = -8; x <= 8; x++) {
  if (x * x + y * y <= 64 && (x - 4) * (x - 4) + (y + 2) * (y + 2) > 40) set(98 + x, 26 + y, C.moon);
}

// cuoricini
const HEART = ["01010", "11111", "11111", "01110", "00100"];
const heart = (ox: number, oy: number) =>
  HEART.forEach((row, y) => [...row].forEach((b, x) => b === "1" && set(ox + x, oy + y, C.pink)));
heart(104, 44);
heart(16, 74);
heart(60, 14);

// fiocco in alto a sinistra, sopra la cornice, con un ciondolo a cuore
const BOW = [
  "0011000000001100",
  "0122100000012210",
  "1222210000122221",
  "1222221111222221",
  "1222222332222221",
  "1222222332222221",
  "1222221111222221",
  "1222210110122221",
  "0122100110012210",
  "0011001001001100",
  "0000011001100000",
  "0000110000110000",
];
const bowColor: Record<string, RGBA> = { "1": C.deep, "2": C.bright, "3": C.glow };
// grande il doppio e appoggiato sull'angolo tagliato
BOW.forEach((row, y) =>
  [...row].forEach((b, x) => {
    if (b === "0") return;
    for (let dy = 0; dy < 2; dy++) for (let dx = 0; dx < 2; dx++) set(2 + x * 2 + dx, 6 + y * 2 + dy, bowColor[b]!);
  }),
);

// foglie in basso a destra
for (const [x, y] of [[112, 92], [113, 93], [114, 94], [110, 96], [111, 97], [115, 98], [116, 99], [113, 100]]) set(x, y, C.cloudLight);

function render(scale: number, size: number, background: RGBA | null): PNG {
  const png = new PNG({ width: size, height: size });
  const offset = Math.floor((size - N * scale) / 2);
  for (let y = 0; y < size; y++) for (let x = 0; x < size; x++) {
    const gx = Math.floor((x - offset) / scale), gy = Math.floor((y - offset) / scale);
    const c = (x >= offset && y >= offset ? get(gx, gy) : null) ?? background;
    const i = (y * size + x) * 4;
    if (c) { png.data[i] = c[0]; png.data[i + 1] = c[1]; png.data[i + 2] = c[2]; png.data[i + 3] = c[3]; }
  }
  return png;
}

const assets = path.join(root, "app/assets");
const write = (file: string, png: PNG) => fs.writeFileSync(path.join(assets, file), PNG.sync.write(png));
const night = hex("#0e0b14");
write("icon.png", render(8, 1024, null)); // icona completa, sfondo trasparente
write("android-icon-foreground.png", render(5, 1024, null)); // dentro la zona sicura delle icone adattive
write("android-icon-background.png", render(1, 1024, night));
const mono = render(5, 1024, null);
for (let i = 0; i < mono.data.length; i += 4) if (mono.data[i + 3]) mono.data.set([255, 255, 255, 255], i);
write("android-icon-monochrome.png", mono);
write("splash-icon.png", render(4, 512, null));
console.log("Icona generata in app/assets");
